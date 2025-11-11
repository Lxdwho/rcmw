/**
 * @brief 提供了QOS配置结构体类以及相关的策略枚举
 * @date 2025.11.10
 */

#ifndef _QOS_PROFILE_H_
#define _QOS_PROFILE_H_

#include "rcmw/serialize/serializable.h"
#include "rcmw/serialize/data_stream.h"

namespace hnu    {
namespace rcmw   {
namespace config {
using namespace serialize;

/* Qos历史策略 */
enum QosHistoryPolicy {
    HISTORY_SYSTEM_DEFAULT  = 0,
    HISTORY_KEEP_LAST       = 1,
    HISTORY_KEEP_ALL        = 2,
};

/* Qos可靠性策略 */
enum QosReliabilityPolicy {
    RELIABILITY_SYSTEM_DEFAULT  = 0,
    RELIABILITY_RELIABLE        = 1,
    RELIABILITY_BEST_EFFORT     = 2,
};

/* Qos持久策略 */
enum QosDurabilityPolicy {
    DURABILITY_SYSTEM_DEFAULT   = 0,
    DURABILITY_TRANSIENT_LOCAL  = 1,
    DURABILITY_VOLATILE         = 2,
};

/* Qos结构体 */
class QosProfile : public Serializable {
public:
    QosHistoryPolicy history = HISTORY_KEEP_LAST;
    uint32_t depth = 2;
    uint32_t mps = 3;
    uint32_t msg_size = 0;
    QosReliabilityPolicy reliability = RELIABILITY_RELIABLE;
    QosDurabilityPolicy durability = DURABILITY_VOLATILE;

    SERIALIZE(history, depth, mps, msg_size, reliability, durability);
};

} // hnu::rcmw::config
}
}

#endif
