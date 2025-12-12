/**
 * @brief attributes填充类实现
 * @date 2025.11.10
 */

#include "rcmw/transport/rtps/attributes_filler.h"
#include "rcmw/transport/qos/qos_profile_conf.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

AttributesFiller::AttributesFiller() {}
AttributesFiller::~AttributesFiller() {}

bool AttributesFiller::FillInWriterAttr(const std::string& channel_name,
    const config::QosProfile& qos, RtpsWriterAttributes* writer_attr) {
    /* 配置Topic */
    writer_attr->tatt.topicName = channel_name; // 话题名称
    writer_attr->tatt.topicDataType = "string"; // 话题数据类型？
    writer_attr->tatt.topicKind = NO_KEY;       // NO_KEY：普通 topic

    /* 配置history */
    writer_attr->hatt.payloadMaxSize = qos.msg_size + 255;  // 单条消息的最大字节数
    writer_attr->hatt.maximumReservedCaches = 50;           // 最多可缓存多少条历史数据

    /* 配置Attributes */
    writer_attr->watt.endpoint.reliabilityKind = BEST_EFFORT;   // 指定 Writer 端点的可靠性模式

    /* 配置qos */
    switch(qos.reliability) {
    case config::QosReliabilityPolicy::RELIABILITY_BEST_EFFORT: // 速度快但不保证完整性
        writer_attr->wqos.m_reliability.kind =  eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
        break;
    case config::QosReliabilityPolicy::RELIABILITY_RELIABLE:    // 通过 NACK 重传机制保证消息可靠送达
        writer_attr->wqos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
    default:
        break;
    }
    
    switch(qos.durability) {
    // 这里是不是有问题？  case config::QosDurabilityPolicy::DURABILITY_TRANSIENT_LOCAL:   
    case config::QosHistoryPolicy::HISTORY_KEEP_LAST:   // 可以接收到之前发过但缓存中仍存在的消息
        writer_attr->wqos.m_durability.kind =  eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS;
        break;
    case config::QosDurabilityPolicy::DURABILITY_VOLATILE:  // 只接收订阅后新发送的数据
        writer_attr->wqos.m_durability.kind = eprosima::fastrtps::VOLATILE_DURABILITY_QOS;
    default:
        break;
    }

    /* 配置topic的history */
    switch (qos.history)
    {
    case config::QosHistoryPolicy::HISTORY_KEEP_LAST:   // 只保留最近 depth 条
        writer_attr->tatt.historyQos.kind = eprosima::fastrtps::KEEP_LAST_HISTORY_QOS;
        break;
    case config::QosHistoryPolicy::HISTORY_KEEP_ALL:    // 尽可能保留全部
        writer_attr->tatt.historyQos.kind = eprosima::fastrtps::KEEP_ALL_HISTORY_QOS;
    default:
        break;
    }

    /**
     * 在 RELIABLE_RELIABILITY_QOS 下需要设置可靠的心跳周期 
     * 在 可靠模式下（RELIABLE），Reader 通过心跳周期确认是否丢包；
     * 心跳太频繁会占用带宽，太慢又可能导致延迟大或丢包不重传。
     * 这里通过 mps（messages per second）倒算心跳间隔，动态调整传输效率。
    */ 
    if(qos.mps != 0) {
        uint64_t mps = qos.mps;
        if(mps > 1024) {
            mps = 1024;
        }
        else if(mps < 64) {
            mps = 64;
        }
        uint64_t fractions = (256ull << 32) / mps;
        uint32_t fraction = fractions & 0xffffffff;
        int32_t seconds = static_cast<int32_t>(fractions >> 32);

        writer_attr->watt.times.heartbeatPeriod.seconds = seconds;
        writer_attr->watt.times.heartbeatPeriod.nanosec = fraction;
    }

    /* 控制历史缓存深度 */
    if(qos.depth != QosProfileConf::QOS_HISTORY_DEPTH_SYSTEM_DEFAULT) {
        writer_attr->tatt.historyQos.depth = static_cast<int32_t>(qos.depth);
    }
    if(writer_attr->tatt.historyQos.depth < 0) return false;
    
    return true;
}

bool AttributesFiller::FillInReaderAttr(const std::string& channel_name, 
    const config::QosProfile& qos, RtpsReaderAttributes* reader_attr) {
    /* 配置Topic */
    reader_attr->tatt.topicName = channel_name;
    reader_attr->tatt.topicDataType = "string";
    reader_attr->tatt.topicKind = NO_KEY;

    reader_attr->hatt.payloadMaxSize = qos.msg_size + 255;

    reader_attr->ratt.endpoint.reliabilityKind = BEST_EFFORT;

    /* 配置qos */
    switch(qos.reliability) {
    case config::QosReliabilityPolicy::RELIABILITY_BEST_EFFORT:
        reader_attr->rqos.m_reliability.kind =  eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
        break;
    case config::QosReliabilityPolicy::RELIABILITY_RELIABLE:
        reader_attr->rqos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
    default:
        break;
    }
    
    switch(qos.durability) {
    case config::QosHistoryPolicy::HISTORY_KEEP_LAST:
        reader_attr->rqos.m_durability.kind =  eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS;
        break;
    case config::QosDurabilityPolicy::DURABILITY_VOLATILE:
        reader_attr->rqos.m_durability.kind = eprosima::fastrtps::VOLATILE_DURABILITY_QOS;
    default:
        break;
    }

    /* 配置topic的history */
    switch (qos.history)
    {
    case config::QosHistoryPolicy::HISTORY_KEEP_LAST:
        reader_attr->tatt.historyQos.kind = eprosima::fastrtps::KEEP_LAST_HISTORY_QOS;
        break;
    case config::QosHistoryPolicy::HISTORY_KEEP_ALL:
        reader_attr->tatt.historyQos.kind = eprosima::fastrtps::KEEP_ALL_HISTORY_QOS;
    default:
        break;
    }

    if(qos.depth != QosProfileConf::QOS_HISTORY_DEPTH_SYSTEM_DEFAULT) {
        reader_attr->tatt.historyQos.depth = static_cast<int32_t>(qos.depth);
    }

    if(reader_attr->tatt.historyQos.depth < 0) return false;
    return true;
}

} // transport
} // rcmw
} // hnu
