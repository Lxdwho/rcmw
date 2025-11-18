/**
 * @brief 角色属性配置
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
    std::string host_name;      // 主机名
    std::string host_ip;        // 主机IP
    int32_t process_id;         // 进程ID

    std::string channel_name;   // 通道/话题名
    uint64_t channel_id;        // 通道/话题ID

    QosProfile qos_profile;     // qos配置
    uint64_t id;                // ID

    std::string node_name;      // 节点名称
    uint64_t node_id;           // 节点ID

    std::string message_type;   // 消息类型

    SERIALIZE(host_name, host_ip, process_id, channel_name, channel_id, qos_profile, id, node_name, node_id, message_type)
};

} // config
} // rcmw
} // hnu

#endif
