/**
 * @brief 订阅者类
 * @date 2026.01.05
 */

#ifndef _RCMW_NODE_SUBSCRIBER_H_
#define _RCMW_NODE_SUBSCRIBER_H_

#include "rcmw/node/subscriber_base.h"
#include "rcmw/transport/tranport.h"
#include "rcmw/transport/receiver/receiver.h"
#include "rcmw/discovery/specific_manager/manager.h"
#include "rcmw/discovery/specific_manager/channel_manager.h"
#include "rcmw/discovery/topology_manager.h"
#include "rcmw/scheduler/scheduler_factory.h"
#include "rcmw/croutine/croutine_factory.h"
#include "rcmw/blocker/blocker.h"

namespace hnu   {
namespace rcmw  {

template <typename M0>
using CallbackFunc = std::function<void(const std::shared_ptr<M0>&)>;

const uint32_t DEFAULT_PENDING_QUEUE_SIZE = 1;

/**
 * @brief 订阅者
 */
template <typename MessageT>
class Subscriber: public SubscriberBase {
public:
    using BlockerPtr = std::unique_ptr<blocker::Blocker<MessageT>>;
    using ReceiverPtr = std::shared_ptr<transport::Receiver<MessageT>>;
    using ChangeConnection = typename discovery::Manager::ChangeConnection;
    using Iterator = typename std::list<std::shared_ptr<MessageT>>::const_iterator;

    explicit Subscriber(const RoleAttributes& role_attr, 
                const CallbackFunc<MessageT>& subscriber_func = nullptr, 
                uint32_t pending_queue_size = DEFAULT_PENDING_QUEUE_SIZE);
    virtual ~Subscriber();
    bool Init() override;
    void Shutdown() override;

    void Observe() override;
    void ClearData() override;
    bool HasReceived() const override;
    bool Empty() const override;
    uint32_t PendingQueueSize() const override;
    double GetDelaySec() const override;
    virtual void Enqueue(const std::shared_ptr<MessageT>& msg);
    virtual void SetHistoryDepth(const uint32_t& depth);
    virtual uint32_t GetHistoryDepth() const;
    virtual Iterator Begin() const { return blocker_->ObservedBegin(); }
    virtual Iterator End() const { return blocker_->ObservedEnd(); }
    virtual std::shared_ptr<MessageT> GetLatestObserved() const;
    virtual std::shared_ptr<MessageT> GetOldestObserved() const;
protected:
    double latest_recv_time_sec_ = -1.0;
    double second_to_latest_recv_time_sec_ = -1.0;
    uint32_t pending_queue_size_;
private:
    void JoinTheTopology();
    void LeaveTheTopology();
    void OnChannelChange(const ChangeMsg& change_msg);

    CallbackFunc<MessageT> subscriber_func_;
    ReceiverPtr receiver_ = nullptr;
    std::string croutine_name_;

    BlockerPtr blocker_;

    ChangeConnection change_conn_;
    discovery::ChannelManagerPtr channel_manager_ = nullptr;
};

template <typename MessageT>
Subscriber<MessageT>::Subscriber(const RoleAttributes& role_attr,
            const CallbackFunc<MessageT>& subscriber_func,
            uint32_t pending_queue_size) 
            : SubscriberBase(role_attr), subscriber_func_(subscriber_func), 
              pending_queue_size_(pending_queue_size) {
    blocker_.reset(new blocker::Blocker<MessageT>(blocker::BlockerAttr(
        role_attr.qos_profile.depth, role_attr.channel_name
    )));
}

template <typename MessageT>
Subscriber<MessageT>::~Subscriber() { Shutdown(); }

template <typename MessageT>
bool Subscriber<MessageT>::Init() {
    if(init_.exchange(true)) return true;
    std::function<void(const std::shared_ptr<MessageT>&)> func;
    if(subscriber_func_ != nullptr) {
        func = [this](const std::shared_ptr<MessageT>& msg){
            this->Enqueue(msg);
            this->subscriber_func_(msg);
        };
    }
    else {
        func = [this](const std::shared_ptr<MessageT>& msg) { this->Enqueue(msg); };
    }

    auto sched = scheduler::Instance();
    croutine_name_ = role_attr_.node_name + "_" + role_attr_.channel_name;

    auto dv = std::make_shared<data::DataVisitor<MessageT>>(role_attr_.channel_id, pending_queue_size_);

    croutine::RoutineFactory factory = croutine::CreateRoutineFactory<MessageT>(std::move(func), dv);

    if(!sched->CreateTask(factory, croutine_name_)) {
        AERROR << "Create Task Failed!";
        init_.store(false);
        return false;
    }

    receiver_ = ReceiverManager<MessageT>::Instance()->GetReceiver(role_attr_);

    channel_manager_ = discovery::TopologyManager::Instance()->channel_manager();

    JoinTheTopology();

    return true;
}

template <typename MessageT>
void Subscriber<MessageT>::Shutdown() {
    if(!init_.exchange(false)) return;
    LeaveTheTopology();
    receiver_ = nullptr;
    channel_manager_ = nullptr;
    if(!croutine_name_.empty()) {
        scheduler::Instance()->RemoveTask(croutine_name_);
    }
}

template <typename MessageT>
void Subscriber<MessageT>::Observe() {
    blocker_->Observe();
}

template <typename MessageT>
void Subscriber<MessageT>::ClearData() {
    blocker_->ClearPublished();
    blocker_->ClearObserved();
}

template <typename MessageT>
bool Subscriber<MessageT>::HasReceived() const {
    return !blocker_->IsPublishedEmpty();
}

template <typename MessageT>
bool Subscriber<MessageT>::Empty() const {
    return blocker_->IsObservedEmpty();
}

template <typename MessageT>
uint32_t Subscriber<MessageT>::PendingQueueSize() const {
    return pending_queue_size_;
}

template <typename MessageT>
double Subscriber<MessageT>::GetDelaySec() const {
    if(latest_recv_time_sec_ < 0) return -1.0;

    if(second_to_latest_recv_time_sec_ < 0) {
        return Time::Now().ToSecond() - latest_recv_time_sec_;
    }

    return std::max((Time::Now().ToSecond() - latest_recv_time_sec_), 
                (latest_recv_time_sec_ - second_to_latest_recv_time_sec_));
}

template <typename MessageT>
void Subscriber<MessageT>::Enqueue(const std::shared_ptr<MessageT>& msg) {
    second_to_latest_recv_time_sec_ = latest_recv_time_sec_;
    latest_recv_time_sec_ = Time::Now().ToSecond();
    blocker_->Publish(msg);
}

template <typename MessageT>
void Subscriber<MessageT>::SetHistoryDepth(const uint32_t& depth) {
    blocker_->set_capacity(depth);
}

template <typename MessageT>
uint32_t Subscriber<MessageT>::GetHistoryDepth() const {
    return static_cast<uint32_t>(blocker_->capacity());
}

template <typename MessageT>
std::shared_ptr<MessageT> Subscriber<MessageT>::GetLatestObserved() const {
    return blocker_->GetLatestObservedPtr();
}

template <typename MessageT>
std::shared_ptr<MessageT> Subscriber<MessageT>::GetOldestObserved() const {
    return blocker_->GetOldestObservedPtr();
}

template <typename MessageT>
void Subscriber<MessageT>::JoinTheTopology() {
    change_conn_ = channel_manager_->AddChangeListener(
        std::bind(&Subscriber<MessageT>::OnChannelChange, this, std::placeholders::_1));
    
    const std::string& channel_name = this->role_attr_.channel_name;
    std::vector<RoleAttributes> publishers;
    channel_manager_->GetWritersOfChannel(channel_name, &publishers);
    for(auto& publisher : publishers) {
        receiver_->Enable(publisher);
    }
    channel_manager_->Join(this->role_attr_, RoleType::ROLE_READER);
}

template <typename MessageT>
void Subscriber<MessageT>::LeaveTheTopology() {
    channel_manager_->RemoveChangeListener(change_conn_);
    channel_manager_->Leave(this->role_attr_, RoleType::ROLE_READER);
}

template <typename MessageT>
void Subscriber<MessageT>::OnChannelChange(const ChangeMsg& change_msg) {
    if(change_msg.role_type != RoleType::ROLE_WRITER) return;
    
    auto& publisher_attr = change_msg.role_attr;
    if(publisher_attr.channel_name != this->role_attr_.channel_name) return;

    auto operate_type = change_msg.operate_type;
    if(operate_type == OperateType::OPT_JOIN) {
        receiver_->Enable(publisher_attr);
    }
    else {
        receiver_->Disable(publisher_attr);
    }
}

} // rcmw
} // hnu

#endif
