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
                uint32_t PendingQueueSize = DEFAULT_PENDING_QUEUE_SIZE);
    virtual ~Subscriber();
    bool Init() override;
    void Shutdown() override;

    void Observe() override;
    void ClearData() override;
    bool HasReceiver() const override;
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
            const CallbackFunc<MessageT>& subscriber_func = nullptr,
            uint32_t pendingQueueSize = DEFAULT_PENDING_QUEUE_SIZE) 
            : SubscriberBase(role_attr, subscriber_func_(subscriber_func), 
              pending_queue_size_(pending_queue_size)) {
    blocker_.reset(new blocker::Blocker<MessageT>(blocker::BlockerAttr(
        role_attr.qos_profile.depth, role_attr.channel_name
    )));
}

template <typename MessageT>
Subscriber<MessageT>::~Subscriber() { Shutdown(); }

template <typename MessageT>
bool Subscriber<MessageT>::Init() {}

template <typename MessageT>
void Subscriber<MessageT>::Shutdown() {}

template <typename MessageT>
void Subscriber<MessageT>::Observe() {}

template <typename MessageT>
void Subscriber<MessageT>::ClearData() {}

template <typename MessageT>
bool Subscriber<MessageT>::HasReceiver() const {}

template <typename MessageT>
bool Subscriber<MessageT>::Empty() const {}

template <typename MessageT>
uint32_t Subscriber<MessageT>::PendingQueueSize() const {}

template <typename MessageT>
double Subscriber<MessageT>::GetDelaySec() const {}

template <typename MessageT>
void Subscriber<MessageT>::Enqueue(const std::shared_ptr<MessageT>& msg) {
    second_to_latest_recv_time_sec_ = latest_recv_time_sec_;
    latest_recv_time_sec_ = Time::Now().ToSecond();
    blocker_->Publish(msg);
}

template <typename MessageT>
void Subscriber<MessageT>::SetHistoryDepth(const uint32_t& depth) {}

template <typename MessageT>
uint32_t Subscriber<MessageT>::GetHistoryDepth() const {}

template <typename MessageT>
std::shared_ptr<MessageT> Subscriber<MessageT>::GetLatestObserved() const {}

template <typename MessageT>
std::shared_ptr<MessageT> Subscriber<MessageT>::GetOldestObserved() const {}

template <typename MessageT>
void Subscriber<MessageT>::JoinTheTopology() {}

template <typename MessageT>
void Subscriber<MessageT>::LeaveTheTopology() {}

template <typename MessageT>
void Subscriber<MessageT>::OnChannelChange(const ChangeMsg& change_msg) {}

} // rcmw
} // hnu

#endif
