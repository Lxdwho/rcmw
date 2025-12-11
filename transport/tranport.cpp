/**
 * @brief Transport顶层
 * @date 2025.12.11
 */

#include "tranport.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

Transport::Transport() {
    CreateParticipant();
    rtps_dispatcher_ = RtpsDispatcher::Instance();
    rtps_dispatcher_->set_participant(participant_);
}

Transport::~Transport() { shutdown(); }

void Transport::shutdown() {
    if(is_shutdown_.exchange(true)) return;
    if(participant_ != nullptr) participant_->Shutdown();
    participant_ = nullptr;
}

void Transport::CreateParticipant() {
    std::string participant_name = common::GlobalData::Instance()->HostName() 
            + "+" + std::to_string(common::GlobalData::Instance()->ProcessId());
    participant_ = std::make_shared<Participant>(participant_name, 11512);
}

} // transport
} // rcmw
} // hnu
