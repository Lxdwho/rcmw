/**
 * @brief participant回调包装
 * @date 2025.12.16
 */

#include "rcmw/discovery/communication/participant_listener.h"
#include "rcmw/logger/log.h"

namespace hnu       {
namespace rcmw      {
namespace discovery {

ParticipantListener::ParticipantListener(const ChangeFunc& callback)
        : callback_(callback) {}

ParticipantListener::~ParticipantListener() {
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = nullptr;
}

void ParticipantListener::onParticipantDiscovery(
        eprosima::fastrtps::rtps::RTPSParticipant* p, 
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) {
    RETURN_IF_NULL(callback_);
    (void)p;
    std::lock_guard<std::mutex> lock(mutex_);
    callback_(info);
}

} // discovery
} // rcmw
} // hnu
