/**
 * @brief participant回调包装
 * @date 2025.12.16
 */

#ifndef _DISCOVERY_PARTIVIPANT_LISTENER_H_
#define _DISCOVERY_PARTIVIPANT_LISTENER_H_

#include "fastrtps/rtps/participant/RTPSParticipantListener.h"
#include <mutex>
#include <functional>

namespace hnu       {
namespace rcmw      {
namespace discovery {

class ParticipantListener : public eprosima::fastrtps::rtps::RTPSParticipantListener {
public:
    using ChangeFunc = std::function<void(
        const eprosima::fastrtps::rtps::ParticipantDiscoveryInfo& info)>;

    explicit ParticipantListener(const ChangeFunc& callback);
    virtual ~ParticipantListener();

    void onParticipantDiscovery(
        eprosima::fastrtps::rtps::RTPSParticipant* p, 
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;
private:
    ChangeFunc callback_;
    std::mutex mutex_;
};

} // discovery
} // rcmw
} // hnu

#endif
