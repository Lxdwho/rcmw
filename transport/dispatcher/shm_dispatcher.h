/**
 * @brief 基于shm的数据分发器
 * @date 2025.12.10
 */

#ifndef _TRANSPORT_SHMDISPATCH_H_
#define _TRANSPORT_SHMDISPATCH_H_

#include "dispatcher.h"
#include "rcmw/transport/shm/segment.h"
#include "rcmw/base/atomic_rw_lock.h"
#include "rcmw/base/rw_lock_guard.h"
#include "rcmw/transport/shm/notifier_base.h"
#include "rcmw/base/macros.h"
#include "rcmw/logger/log.h"
#include "rcmw/serialize/serializable.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

class ShmDispatcher;
using ShmDispatcherPtr = ShmDispatcher*;

class ShmDispatcher : public Dispatcher {
public:
    using SegmentContainer = std::unordered_map<uint64_t, SegmentPtr>;
    virtual ~ShmDispatcher();
    void Shutdown() override;
    
    template<typename M>
    void AddListener(const RoleAttributes& self_attr,
                    const MessageListener<M>& listener);
                    
    template<typename M>
    void AddListener(const RoleAttributes& self_attr,
                    const RoleAttributes& opposite_attr,
                    const MessageListener<M>& listener);
private:
    void AddSegment(const RoleAttributes& self_attr);
    void ReadMessage(uint64_t channel_id, uint32_t block_index);
    void OnMessage(uint64_t channel_id, const std::shared_ptr<ReadableBlock>& rb,
                    const MessageInfo& msg_info);
    void ThreadFunc();
    bool Init();

    uint64_t host_id_;
    SegmentContainer segments_;
    std::unordered_map<uint64_t, uint32_t> previous_indexs_;
    AtomicRWLock segments_lock_;
    std::thread thread_;
    NotifierPtr notifier_;
    // 单例类
    DECLARE_SINGLETON(ShmDispatcher)
};

template<typename M>
void ShmDispatcher::AddListener(const RoleAttributes& self_attr,
                const MessageListener<M>& listener) {
    auto listener_adapter = [listener](const std::shared_ptr<ReadableBlock>& rb, 
                                        const MessageInfo& msg_info) {
        auto msg = std::make_shared<M>();
        serialize::DataStream ds(reinterpret_cast<char*>(rb->buf), rb->block->msg_size());
        ds >> *msg;
        listener(msg, msg_info);
    };
    Dispatcher::AddListener<ReadableBlock>(self_attr, listener_adapter);
    AddSegment(self_attr);
}

template<typename M>
void ShmDispatcher::AddListener(const RoleAttributes& self_attr,
                const RoleAttributes& opposite_attr,
                const MessageListener<M>& listener) {
    auto listener_adapter = [listener](const std::shared_ptr<ReadableBlock>& rb,
                                        const MessageInfo& msg_info) {
        auto msg = std::make_shared<M>();
        serialize::DataStream ds(reinterpret_cast<char*>(rb->buf), rb->block->msg_size());
        ds >> *msg;
        listener(msg, msg_info);
    };
    Dispatcher::AddListener<ReadableBlock>(self_attr, opposite_attr, listener_adapter);
    AddSegment(self_attr);
}

} // transport
} // rcmw
} // hnu

#endif
