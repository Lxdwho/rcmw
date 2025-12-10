/**
 * @brief 数据分发器
 * @date 2025.12.10
 */

#ifndef _TRANSPORT_DISPATCH_H_
#define _TRANSPORT_DISPATCH_H_

#include <atomic>
#include <functional>
#include "rcmw/config/RoleAttributes.h"
#include "rcmw/base/atomic_hash_map.h"
#include "rcmw/base/atomic_rw_lock.h"
#include "rcmw/transport/message/message_info.h"
#include "rcmw/transport/message/listener_handler.h"
#include "rcmw/common/global_data.h"
#include "rcmw/logger/log.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

using namespace common;
using hnu::rcmw::base::AtomicHashMap;
using hnu::rcmw::base::AtomicRWLock;

using namespace config;

class Dispatcher;
using DispatcherPtr = std::shared_ptr<Dispatcher>;

template<typename M>
using MessageListener = std::function<void(const std::shared_ptr<M>&, 
                        const MessageInfo)>;

class Dispatcher {
public:
    Dispatcher();
    virtual ~Dispatcher();
    virtual void Shutdown();

    template<typename M>
    void AddListener(const RoleAttributes& self_attr, 
                    const MessageListener<M>& listener);
    
    template<typename M>
    void AddListener(const RoleAttributes& self_attr, 
                    const RoleAttributes& opposite_attr, 
                    const MessageListener<M>& listener);
    
    template<typename M>
    void RemoveListener(const RoleAttributes& self_attr);
    
    template<typename M>
    void RemoveListener(const RoleAttributes& self_attr, 
                        const RoleAttributes& opposite_attr);
    
    bool HasChannel(uint64_t channel_id);

protected:
    std::atomic<bool> is_shutdown_;
    AtomicHashMap<uint64_t, ListenerHandlerBasePtr> msg_listeners_;
    base::AtomicRWLock rw_lock_;
};

template<typename M>
void Dispatcher::AddListener(const RoleAttributes& self_attr, 
                            const MessageListener<M>& listener) {
    if(is_shutdown_.load()) return;
    uint64_t channel_id = self_attr.channel_id;
    std::shared_ptr<ListenerHandler<M>> handler;
    ListenerHandlerBasePtr* handler_base = nullptr;
    if(msg_listeners_.Get(channel_id, &handler_base)) {
        handler = std::dynamic_pointer_cast<ListenerHandler<M>>(*handler_base);
        if(handler == nullptr) {
            AERROR << "please ensure that readers with the same channel["
                   << self_attr.channel_name
                   << "] in the same process have the same message type."
            return;
        }
    }
    else {
        ADEBUG << "new reader for channel:" << GlobalData::GetChannelById(channel_id);
        handler.reset(new ListenerHandler<M>());
        msg_listeners_.Set(channel_id, handler);
    }
    handler->Connect(self_attr.id, listener);
}

template<typename M>
void Dispatcher::AddListener(const RoleAttributes& self_attr,
                            const RoleAttributes& opposite_attr, 
                            const MessageListener<M>& listener) {
    if(is_shutdown_.load()) return;
    uint64_t channel_id = self_attr.channel_id;
    std::shared_ptr<ListenerHandler<M>> handler;
    ListenerHandlerBasePtr* handler_base = nullptr;
    if(msg_listeners_.Get(channel_id, &handler_base)) {
        handler = std::dynamic_pointer_cast<ListenerHandler<M>>(*handler_base);
        if(handler == nullptr) {
            AERROR << "please ensure that readers with the same channel["
                   << self_attr.channel_name
                   << "] in the same process have the same message type."
            return;
        }
    }
    else {
        ADEBUG << "new reader for channel:" << GlobalData::GetChannelById(channel_id);
        handler.reset(new ListenerHandler<M>());
        msg_listeners_.Set(channel_id, handler);
    }
    handler->Connect(self_attr, opposite_attr, listener);
}

template<typename M>
void Dispatcher::RemoveListener(const RoleAttributes& self_attr) {
    if(is_shutdown_.load()) return;
    uint64_t channle_id = self_attr.channel_id;
    ListenerHandlerBasePtr* handler_base = nullptr;
    if(msg_listeners_.Get(channle_id, &handler_base))
        (*handler_base)->Disconnect(self_attr.id);
}

template<typename M>
void Dispatcher::RemoveListener(const RoleAttributes& self_attr,
                                const RoleAttributes& opposite_attr) {
    if(is_shutdown_.load()) return;
    uint64_t channle_id = self_attr.channel_id;
    ListenerHandlerBasePtr* handler_base = nullptr;
    if(msg_listeners_.Get(channle_id, &handler_base))
        (*handler_base)->Disconnect(self_attr.id, opposite_attr.id);
}

} // transport
} // rcmw
} // hnu

#endif
