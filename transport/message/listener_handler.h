/**
 * @brief 
 * @date 2025.12.02
 */

#ifndef _TRANSPORT_LISTENER_HANDLER_H_
#define _TRANSPORT_LISTENER_HANDLER_H_

#include <unordered_map>
#include <functional>
#include <string>
#include <memory>
#include "rcmw/logger/log.h"
#include "rcmw/base/signal_slot.h"
#include "rcmw/base/atomic_rw_lock.h"
#include "rcmw/serialize/data_stream.h"
#include "rcmw/transport/message/message_info.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

using hnu::rcmw::base::AtomicRWLock;
using hnu::rcmw::base::ReadLockGuard;
using hnu::rcmw::base::WriteLockGuard;

class ListenerHandlerBase;
using ListenerHandlerBasePtr = std::shared_ptr<ListenerHandlerBase>;

class ListenerHandlerBase {
public:
    ListenerHandlerBase() {}
    virtual ~ListenerHandlerBase() {}

    virtual void Disconnect(uint64_t self_id) = 0;
    virtual void Disconnect(uint64_t self_id, uint64_t oppo_id) = 0;
    virtual void RunFromString(const std::string& str, 
                                const MessageInfo& msg_info) = 0;
protected:
    bool is_raw_message_ = false;
};

template<typename M>
class ListenerHandler : public ListenerHandlerBase {
public:
    using Message = std::shared_ptr<M>;
    using MessageSignal = base::Signal<const Message&, const MessageInfo&>;
    using Listener = std::function<void(const Message&, const MessageInfo&)>;
    using MessageConnection = base::Connection<const Message&, 
                const MessageInfo&>;
    using ConnectionMap = std::unordered_map<uint64_t, MessageConnection>;

    ListenerHandler() {}
    virtual ~ListenerHandler() {}

    void Connect(uint64_t self_id, const Listener& listener);
    void Connect(uint64_t self_id, uint64_t oppo_id, const Listener& listener);

    void Disconnect(uint64_t self_id) override;
    void Disconnect(uint64_t self_id, uint64_t oppo_id) override;

    void Run(const Message& msg, const MessageInfo& msg_info);
    void RunFromString(const std::string& str, const MessageInfo& msg_info) override;
private:
    using SignalPtr = std::shared_ptr<MessageSignal>;
    using MessageSignalMap = std::unordered_map<uint64_t, SignalPtr>;

    MessageSignal signal_;          // 信号
    ConnectionMap signal_conns_;    // 存储该信号的连接关系：umap<self_id, connection>
    MessageSignalMap signals_;      // 信号umap
    std::unordered_map<uint64_t, ConnectionMap> signals_conns_;
    base::AtomicRWLock rw_lock_;
};

template<typename M>
void ListenerHandler<M>::Connect(uint64_t self_id, const Listener& Listener) {
    auto connection = signal_.Connect(Listener);
    if(!connection.IsConnected()) return;
    WriteLockGuard<AtomicRWLock> lock(rw_lock_);
    signal_conns_[self_id] = connection;
}

template<typename M>
void ListenerHandler<M>::Connect(uint64_t self_id, uint64_t oppo_id, const Listener& Listener) {
    WriteLockGuard<AtomicRWLock> lock(rw_lock_);
    if(signals_.find(oppo_id) == signals_.end())
        signals_[oppo_id] = std::make_shared<MessageSignal>();
    auto connection = signals_[oppo_id]->Connect(Listener);
    if(!connection.IsConnected())
        ADEBUG << oppo_id << " " << self_id << " connect failed!";
    if(signals_conns_.find(oppo_id) == signals_conns_.end())
        signals_conns_[oppo_id] = ConnectionMap();
    signals_conns_[oppo_id][self_id] = connection;
}

template<typename M>
void ListenerHandler<M>::Disconnect(uint64_t self_id) {
    WriteLockGuard<AtomicRWLock> lock(rw_lock_);
    if(signal_conns_.find(self_id) == signal_conns_.end()) return;
    signal_conns_[self_id].Disconnect();
    signal_conns_.erase(self_id);
}

template<typename M>
void ListenerHandler<M>::Disconnect(uint64_t self_id, uint64_t oppo_id) {
    WriteLockGuard<AtomicRWLock> lock(rw_lock_);
    if(signals_conns_.find(oppo_id) == signals_conns_.end()) return;
    if(signals_conns_[oppo_id].find(self_id) == signals_conns_[oppo_id].end()) return;
    signals_conns_[oppo_id][self_id].Disconnect();
    signals_conns_[oppo_id].erase(self_id);
}

template<typename M>
void ListenerHandler<M>::Run(const Message& msg, const MessageInfo& msg_info) {
    signal_(msg, msg_info);

    uint64_t oppo_id = msg_info.sender_id().HashValue();
    WriteLockGuard<AtomicRWLock> lock(rw_lock_);
    if(signals_.find(oppo_id) == signals_.end()) return;
    (*signals_[oppo_id])(msg, msg_info);
}

template<typename M>
void ListenerHandler<M>::RunFromString(const std::string& str, 
                                       const MessageInfo& msg_info) {
    // auto msg = std::make_shared<MessageT>();
    // serialize::DataStream ds(str);
    // ds >> *msg;

    // Run(msg,msg_info);
    AERROR << "RunFromString Error!";
}

} // transport
} // rcmw
} // hnu

#endif
