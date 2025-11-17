/**
 * @brief 
 * @date 2025.11.16
 */

#ifndef _ROLEATTRIBUTES_H_
#define _ROLEATTRIBUTES_H_

#include "qos_profile.h"
#include "rcmw/serialize/serializable.h"
#include "rcmw/serialize/data_stream.h"
#include <string>

namespace hnu    {
namespace rcmw   {
namespace config {

using namespace hnu::rcmw::serialize;

struct RoleAttributes : public Serializable {
    std::string host_name;
    std::string host_ip;
    int32_t process_id;

    std::string channel_name;
    uint64_t channel_id;

    QosProfile qos_profile;
    uint64_t id;

    std::string node_name;
    uint64_t node_id;

    std::string message_type;

    SERIALIZE(host_name, host_ip, process_id, channel_name, channel_id, qos_profile, id, node_name, node_id, message_type)
};

} // config
} // rcmw
} // hnu

#endif
