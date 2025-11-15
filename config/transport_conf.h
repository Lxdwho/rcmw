/**
 * @brief 待看
 * @date 2025.11.14
 */

#ifndef _TRANSPORT_CONF_H_
#define _TRANSPORT_CONF_H_

#include <string>

namespace hnu    {
namespace rcmw   {
namespace config {

enum OptionalMode {
    HYBRID = 0,
    INTRA  = 1,
    SHM    = 2,
    RTPS   = 3,
};

struct ShmMulticastLocator {
    std::string ip;
    uint32_t port;
};

struct ShmConf {
    std::string notifier_type;
    std::string shm_type;
    ShmMulticastLocator shm_locator;
};

struct CommunicationMode {
    OptionalMode same_proc = INTRA;
    OptionalMode diff_proc = SHM;
    OptionalMode diff_host = RTPS;
};

struct TransprotConf {
    ShmConf shm_conf;
    CommunicationMode communication_mode;
};

}
}
}

#endif
