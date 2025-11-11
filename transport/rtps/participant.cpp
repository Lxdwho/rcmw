/**
 * @brief participant 构建类实现
 * @date 2025.11.11
 */

#include "participant.h"
#include "fastrtps/rtps/RTPSDomain.h"
#include "fastrtps/rtps/attributes/RTPSParticipantAttributes.h"
#include "fastrtps/rtps/participant/RTPSParticipant.h"
#include "rcmw/common/environment.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

Participant::Participant(const std::string& name, int send_port, 
        eprosima::fastrtps::rtps::RTPSParticipantListener* listener = nullptr)
      : shutdown_(false), 
        name_(name), 
        send_port_(send_port), 
        fastrtps_participant_(nullptr) {}

Participant::~Participant() {}

void Participant::Shutdown() {
    if(shutdown_.exchange(true)) return ;
    std::lock_guard<std::mutex> lock(mutex_);
    if(fastrtps_participant_ != nullptr) {
        eprosima::fastrtps::rtps::RTPSDomain::removeRTPSParticipant(fastrtps_participant_);
        fastrtps_participant_ = nullptr;
        listener_ = nullptr;
    }
}

eprosima::fastrtps::rtps::RTPSParticipant* Participant::fastrtps_participant() {
    if(shutdown_.load()) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if(fastrtps_participant_ != nullptr) {
        return fastrtps_participant_;
    }
    CreateFastRtpsParticipant(name_, send_port_, listener_);
    return fastrtps_participant_;
}

void Participant::CreateFastRtpsParticipant(
        const std::string& name, int send_port, 
        eprosima::fastrtps::rtps::RTPSParticipantListener* listener) {
    uint32_t domain_id = 0;

    RTPSParticipantAttributes PParam;
    PParam.builtin.use_WriterLivelinessProtocol = true;
    PParam.builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
    PParam.builtin.discovery_config.discoveryProtocol = eprosima::fastrtps::rtps::DiscoveryProtocol::SIMPLE;
    PParam.setName(name.c_str());

    Locator_t loc;
    loc.kind = LOCATOR_KIND_UDPv4;

    std::string ip_env("127.0.0.1");
    const char* ip_val = std::getenv("RCMW_IP");
    if(ip_val != nullptr) {
        ip_env = ip_val;
        if(ip_env.empty()) {
            AERROR << "invalid RCMW_IP (an empty string)";
            return;
        }
    }
    ADEBUG << "rcmw ip: " << ip_env;

    IPLocator::setIPv4(loc, ip_env);
    /*  */
    PParam.defaultUnicastLocatorList.push_back(loc);
    PParam.builtin.metatrafficUnicastLocatorList.push_back(loc);

    /*  */
    IPLocator::setIPv4(loc, "239.255.0.1");
    PParam.builtin.metatrafficMulticastLocatorList.push_back(loc);
    PParam.defaultMulticastLocatorList.push_back(loc);

    /*  */
    fastrtps_participant_ = RTPSDomain::createParticipant(domain_id, PParam, listener);
}

} // transport
} // rcmw
} // hnu
