/**
 * @brief 
 * @date 2025.11.18
 */

#ifndef _TOPOLOGY_CHANGE_H_
#define _TOPOLOGY_CHANGE_H_

#include <cstdint>
#include "rcmw/serialize/serializable.h"
#include "rcmw/serialize/data_stream.h"
#include "RoleAttributes.h"

namespace hnu    {
namespace rcmw   {
namespace config {

using namespace serialize;
//改变的类型
enum ChangeType{
    CHANGE_NODE        = 1,
    CHANGE_CHANNEL     = 2,
    CHANGE_SERVICE     = 3,
    CHANGE_PARTICIPANT = 4,
};

//一个角色的动作
enum OperateType {
    OPT_JOIN  = 1,
    OPT_LEAVE = 2,
};

//通信平面中角色的类型
enum RoleType {
    ROLE_NODE         = 1,      // Node
    ROLE_WRITER       = 2,      // Publisher
    ROLE_READER       = 3,      // Subscriber
    ROLE_SERVER       = 4,      // 
    ROLE_CLIENT       = 5,
    ROLE_PARTICIPANT  = 6,
};

struct ChangeMsg : public Serializable
{
    uint64_t timestamp;  
    ChangeType change_type ;
    OperateType operate_type;
    RoleType role_type;
    RoleAttributes role_attr;

    SERIALIZE(timestamp,change_type,operate_type,role_type,role_attr)
};


} // config
} // rcmw
} // hnu

#endif
