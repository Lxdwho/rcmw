/**
 * @brief 提供几种情况下的Qos策略
 * @date 2025.11.10
 */

#include "qos_profile_conf.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

QosProfileConf::QosProfileConf() {}

QosProfileConf::~QosProfileConf() {}

/* Qos策略结构体创建 */
QosProfile QosProfileConf::CreateQosProfile(const QosHistoryPolicy& history,
                                    uint32_t depth, uint32_t mps,  
                                    const QosReliabilityPolicy& reliability,
                                    const QosDurabilityPolicy& durability) {
    QosProfile qos_profile;                 // QOS策略结构体
    qos_profile.history = history;          // 历史消息缓存策略
    qos_profile.reliability = reliability;  // 可靠性策略
    qos_profile.durability = durability;    // 持久性策略
    qos_profile.mps = mps;                  // 每个发布者的最大样本数
    qos_profile.depth = depth;              // 缓存深度
    return qos_profile;
}

const uint32_t QosProfileConf::QOS_HISTROY_DEPTH_SYSTEM_DEFAULT = 0;
const uint32_t QosProfileConf::QOS_MPS_SYSTEM_DEFAULT = 0;

/* 默认Qos策略 */
const QosProfile QosProfileConf::QOS_PROFILE_DEFAULT = CreateQosProfile(
    QosHistoryPolicy::HISTORY_KEEP_LAST, 
    1, 
    QOS_MPS_SYSTEM_DEFAULT, 
    QosReliabilityPolicy::RELIABILITY_RELIABLE, 
    QosDurabilityPolicy::DURABILITY_VOLATILE
);

/* 传感器数据策略 */
const QosProfile QosProfileConf::QOS_PROFILE_SENSOR_DATA = QosProfileConf::CreateQosProfile(
    QosHistoryPolicy::HISTORY_KEEP_LAST,
    5,
    QOS_MPS_SYSTEM_DEFAULT, 
    QosReliabilityPolicy::RELIABILITY_BEST_EFFORT, 
    QosDurabilityPolicy::DURABILITY_VOLATILE
);

/* 参数策略 */
const QosProfile QosProfileConf::QOS_PROFILE_PARAMETERS = QosProfileConf::CreateQosProfile(
    QosHistoryPolicy::HISTORY_KEEP_LAST,
    1000,
    QOS_MPS_SYSTEM_DEFAULT, 
    QosReliabilityPolicy::RELIABILITY_RELIABLE, 
    QosDurabilityPolicy::DURABILITY_VOLATILE
);

/* 业务/服务策略 */
const QosProfile QosProfileConf::QOS_PROFILE_SERVICES_DEFAULT = QosProfileConf::CreateQosProfile(
    QosHistoryPolicy::HISTORY_KEEP_LAST,
    10,
    QOS_MPS_SYSTEM_DEFAULT, 
    QosReliabilityPolicy::RELIABILITY_RELIABLE, 
    QosDurabilityPolicy::DURABILITY_TRANSIENT_LOCAL
);

/* 参数事件策略 */
const QosProfile QosProfileConf::QOS_PROFILE_PARAM_EVENT = QosProfileConf::CreateQosProfile(
    QosHistoryPolicy::HISTORY_KEEP_LAST,
    1000,
    QOS_MPS_SYSTEM_DEFAULT, 
    QosReliabilityPolicy::RELIABILITY_RELIABLE, 
    QosDurabilityPolicy::DURABILITY_VOLATILE
);

/* 系统默认策略 */
const QosProfile QosProfileConf::QOS_PROFILE_SYSTEM_DEFAULT = QosProfileConf::CreateQosProfile(
    QosHistoryPolicy::HISTORY_SYSTEM_DEFAULT, 
    QosProfileConf::QOS_HISTROY_DEPTH_SYSTEM_DEFAULT,
    QOS_MPS_SYSTEM_DEFAULT, 
    QosReliabilityPolicy::RELIABILITY_RELIABLE, 
    QosDurabilityPolicy::DURABILITY_TRANSIENT_LOCAL
);

/* 静态TF坐标策略 */
const QosProfile QosProfileConf::QOS_PROFILE_TF_STATIC = QosProfileConf::CreateQosProfile(
    QosHistoryPolicy::HISTORY_KEEP_ALL, 
    10,
    QOS_MPS_SYSTEM_DEFAULT, 
    QosReliabilityPolicy::RELIABILITY_RELIABLE, 
    QosDurabilityPolicy::DURABILITY_TRANSIENT_LOCAL
);

/* ？ */
const QosProfile QosProfileConf::QOS_PROFILE_TOPO_CHANGE = QosProfileConf::CreateQosProfile(
    QosHistoryPolicy::HISTORY_KEEP_ALL, 
    10,
    QOS_MPS_SYSTEM_DEFAULT, 
    QosReliabilityPolicy::RELIABILITY_RELIABLE, 
    QosDurabilityPolicy::DURABILITY_TRANSIENT_LOCAL
);

} // transport
} // rcmw
} // hnu
