/**
 * @brief 订阅者基类
 * @date 2026.01.05
 */

#ifndef _RCMW_NODE_SUBSCRIBERBASE_H_
#define _RCMW_NODE_SUBSCRIBERBASE_H_

#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include "rcmw/config/RoleAttributes.h"
#include "rcmw/common/macros.h"
#include "rcmw/transport/tranport.h"
#include "rcmw/transport/receiver/receiver.h"
#include "rcmw/data/data_dispatcher.h"
#include "rcmw/event/perf_event_cache.h"

namespace hnu   {
namespace rcmw  {

using namespace config;

/**
 * @brief 订阅者基类
 */
class SubscriberBase {
public:
    explicit SubscriberBase(const RoleAttributes& role_attr) 
            : role_attr_(role_attr), init_(false) {}
    virtual ~SubscriberBase() {}

    virtual bool Init() = 0;
    virtual void Shutdown() = 0;

    virtual void ClearData() = 0;
    virtual void Observe() = 0;
    virtual bool Empty() const = 0;
    virtual bool HasReceived() const = 0;
    virtual double GetDelaySec() const = 0;
    virtual uint32_t PendingQueueSize() const = 0;
    virtual void GetPublishers(std::vector<RoleAttributes>* publishers) {}

    const std::string& GetChannelName() const {
        return role_attr_.channel_name;
    }

    uint64_t ChannelId() const { return role_attr_.channel_id; }

    const QosProfile& GetQosProfile() const {
        return role_attr_.qos_profile;
    }

    bool IsInit() const { return init_.load(); }
protected:
    RoleAttributes role_attr_;
    std::atomic<bool> init_;
};

/**
 * @brief 订阅者管理类
 */
template <typename MessageT>
class ReceiverManager {
public:
    ~ReceiverManager() { receiver_map_.clear(); }
    auto GetReceiver(const RoleAttributes& role_attr)
            ->typename std::shared_ptr<transport::Receiver<MessageT>>;
private:
    std::unordered_map<std::string, 
            typename std::shared_ptr<transport::Receiver<MessageT>>> receiver_map_;
    std::mutex receiver_map_mutex_;
    DECLARE_SINGLETON(ReceiverManager<MessageT>) 
};

template <typename MessageT>
ReceiverManager<MessageT>::ReceiverManager() {}

template <typename MessageT>
auto ReceiverManager<MessageT>::GetReceiver(const RoleAttributes& role_attr)
        ->typename std::shared_ptr<transport::Receiver<MessageT>> {
    std::lock_guard<std::mutex> lg(receiver_map_mutex_);
    const std::string& channel_name = role_attr.channel_name;
    if(receiver_map_.count(channel_name) == 0) {
        receiver_map_[channel_name] = transport::Transport::Instance()
                                    ->CreateReceiver<MessageT>(role_attr, 
            [](const std::shared_ptr<MessageT>& msg, 
                const transport::MessageInfo& msg_info, 
                const RoleAttributes& subscriber_attr) {
                data::DataDispatcher<MessageT>::Instance()->Dispatch(
                    subscriber_attr.channel_id, msg);
            }
        );
    }
    return receiver_map_[channel_name];
}

} // rcmw
} // hnu

#endif
