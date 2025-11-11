/**
 * @brief participant 构建类
 * @date 2025.11.11
 */

#ifndef _PARTICIPANT_H_
#define _PARTICIPANT_H_

#include "fastrtps/rtps/participant/RTPSParticipant.h"
#include <atomic>
#include <mutex>
#include <string>

namespace hnu       {
namespace rcmw      {
namespace transport {

class Participant;
using ParticipantPtr = std::shared_ptr<Participant>;

class Participant {
public:
    Participant(const std::string& name, int send_port, 
        eprosima::fastrtps::rtps::RTPSParticipantListener* listener = nullptr);
    virtual ~Participant();
    void Shutdown();
    bool is_shutdown() const { return shutdown_.load(); }

    eprosima::fastrtps::rtps::RTPSParticipant* fastrtps_participant();
private:
    void CreateFastRtpsParticipant(
        const std::string& name, int send_port, 
        eprosima::fastrtps::rtps::RTPSParticipantListener* listener);

    std::atomic<bool> shutdown_;
    std::string name_;              //RTPSParticipant name
    int send_port_;
    eprosima::fastrtps::rtps::RTPSParticipant* fastrtps_participant_;
    eprosima::fastrtps::rtps::RTPSParticipantListener* listener_;
    std::mutex mutex_;
};
}
}
}

#endif
