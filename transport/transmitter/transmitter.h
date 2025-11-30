/**
 * @brief Transmitter基类
 * @date 2025.11.16
 */

#ifndef _TRANSMITTER_H_
#define _TRANSMITTER_H_

#include <cstdint>
#include <memory>
#include "rcmw/transport/common/endpoint.h"
#include "rcmw/transport/message/message_info.h"
#include "rcmw/event/perf_event_cache.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

using namespace event;

template<typename M>
class Transmitter : public Endpoint {
public:
    using MessagePtr = std::shared_ptr<M>;
    explicit Transmitter(const RoleAttributes& attr);
    virtual ~Transmitter();

    virtual void Enable() = 0;
    virtual void Disable() = 0;

    virtual void Enable(const RoleAttributes& attr);
    virtual void Disable(const RoleAttributes& attr);

    virtual bool Transmit(const MessagePtr& msg);
    virtual bool Transmit(const MessagePtr& msg, const MessageInfo& msg_info) = 0;

    uint64_t NextSeqNum() { return ++seq_num_; }
    uint64_t seq_num() const { return seq_num_; }

protected:
    uint64_t seq_num_;
    MessageInfo msg_info_;
};

template<typename M>
Transmitter<M>::Transmitter(const RoleAttributes& attr) : Endpoint(attr), seq_num_(0) {
    msg_info_.set_sender_id(this->id_);
    msg_info_.set_seq_num(this->seq_num_);
}

template<typename M>
Transmitter<M>::~Transmitter() {}

template<typename M>
void Transmitter<M>::Enable(const RoleAttributes& attr) {
    (void)attr;
    Enable();
}

template<typename M>
void Transmitter<M>::Disable(const RoleAttributes& attr) {
    (void)attr;
    Disable();
}

template<typename M>
bool Transmitter<M>::Transmit(const MessagePtr& msg) {
    msg_info_.set_seq_num(NextSeqNum());
    PerfEventCache::Instance()->AddTransportEvent(TransPerf::TRANSMIT_BEGIN, 
        attr_.channel_id, msg_info_.seq_num());
    return Transmit(msg, msg_info_);
}

} // transport
} // rcmw
} // hnu

#endif
