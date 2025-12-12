/**
 * @brief Transport顶层
 * @date 2025.12.11
 */

#ifndef _TRANSPORT_TRANSPORT_H_
#define _TRANSPORT_TRANSPORT_H_

#include "rtps/participant.h"
#include "rcmw/common/macros.h"
#include "rcmw/config/transport_conf.h"
#include "rcmw/config/RoleAttributes.h"
#include "rcmw/transport/transmitter/transmitter.h"
#include "rcmw/transport/transmitter/rtps_transmitter.h"
#include "rcmw/transport/transmitter/shm_transmitter.h"
#include "rcmw/transport/receiver/receiver.h"
#include "rcmw/transport/receiver/shm_receiver.h"
#include "rcmw/transport/receiver/rtps_receiver.h"
#include "rcmw/transport/dispatcher/rtps_dispatcher.h"
#include "rcmw/logger/log.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

class Transport {
public:
    virtual ~Transport();
    void shutdown();

    template<typename M>
    auto CreateTransmitter(const RoleAttributes& attr,
                           const OptionalMode& mode = OptionalMode::RTPS)->
                           typename std::shared_ptr<Transmitter<M>>;
    
    template<typename M>
    auto CreateReceiver(const RoleAttributes& attr,
                        const typename Receiver<M>::MessageListener& msg_listener,
                        const OptionalMode& mode = OptionalMode::RTPS)->
                        typename std::shared_ptr<Receiver<M>>;
    
    ParticipantPtr participant() const { return participant_; }

private:
    void CreateParticipant();
    ParticipantPtr participant_ = nullptr;
    std::atomic<bool> is_shutdown_ = { false };
    RtpsDispatcherPtr rtps_dispatcher_ = nullptr;
    DECLARE_SINGLETON(Transport)
};

template<typename M>
auto Transport::CreateTransmitter(const RoleAttributes& attr,
                        const OptionalMode& mode)->
                        typename std::shared_ptr<Transmitter<M>> {
    if(is_shutdown_.load()) {
        AINFO << "transitter has been shutdown.";
        return nullptr;
    }
    std::shared_ptr<Transmitter<M>> transmitter = nullptr;
    RoleAttributes modified_attr = attr;

    switch(mode) {
        case OptionalMode::SHM:
            transmitter = std::make_shared<ShmTransmitter<M>>(modified_attr);
            break;
        default:
            transmitter = std::make_shared<RtpsTransmitter<M>>(modified_attr, participant());
            break;
    }

    RETURN_VAL_IF_NULL(transmitter, nullptr)
    if(mode != OptionalMode::HYBRID) {
        ADEBUG << "transmitter Enable";
        transmitter->Enable();
    }
    AINFO << "CreateTransmitter Sucess";
    return transmitter;
}

template<typename M>
auto Transport::CreateReceiver(const RoleAttributes& attr,
                    const typename Receiver<M>::MessageListener& msg_listener,
                    const OptionalMode& mode)->
                    typename std::shared_ptr<Receiver<M>> {
    if(is_shutdown_.load()) {
        AINFO << "transitter has been shutdown.";
        return nullptr;
    }

    std::shared_ptr<Receiver<M>> receiver = nullptr;
    RoleAttributes modified_attr = attr;
    ADEBUG << "receive mode: " << mode;
    switch(mode){
        case OptionalMode::RTPS:
            receiver = std::make_shared<ShmReceiver<M>>(modified_attr, msg_listener);
            break;
        default:
            receiver = std::make_shared<RtpsReceiver<M>>(modified_attr, msg_listener);
            break;
    }
    RETURN_VAL_IF_NULL(receiver, nullptr);
    if(mode != OptionalMode::HYBRID) {
        receiver->Enable();
    }
    return receiver;
}

} // transport
} // rcmw
} // hnu

#endif
