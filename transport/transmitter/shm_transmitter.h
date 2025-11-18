/**
 * @brief 基于shm的transmitter
 * @date 2025.11.17
 */

#ifndef _SHM_TRANSMITTER_H_
#define _SHM_TRANSMITTER_H_

#include "transmitter.h"
#include "rcmw/transport/shm/segment_factory.h"
#include "rcmw/transport/shm/notifier_factory.h"
#include "rcmw/transport/common/endpoint.h"
#include "rcmw/logger/log.h"
#include <cstring>

namespace hnu       {
namespace rcmw      {
namespace transport {

template <typename M>
class ShmTransmitter : public Transmitter<M> {
public:
    /* 模板消息智能指针 */
    using MessagePtr = std::shared_ptr<M>;

    explicit ShmTransmitter(const RoleAttributes& attr);
    virtual ~ShmTransmitter();

    void Enable() override;
    void Disable() override;

    bool Transmit(const MessagePtr& msg, const MessageInfo& info) override;
private:
    bool Transmit(const M& msg, const MessageInfo& msg_info);
    
    SegmentPtr segment_;    // 准备写的共享内存
    uint64_t channel_id_;   // 发送的通道、话题
    uint64_t host_id_;      // 发送主机ID
    NotifierPtr notifier_;  // 发送notifier
};

/**
 * @tparam M: 发送的消息类型
 * @param attr:发送者角色属性
 * @brief 构造函数，首先构造父类，随后对类内维护的变量赋初值
 */
template <typename M>
ShmTransmitter<M>::ShmTransmitter(const RoleAttributes& attr) 
    : Transmitter<M>(attr), 
      segment_(nullptr), 
      channel_id_(attr.channel_id),
      notifier_(nullptr)  
      { host_id_ = common::Hash(attr.host_ip); }

/**
 * @tparam M: 发送的消息类型
 * @brief 析构函数，直接调用Disable
 */
template <typename M>
ShmTransmitter<M>::~ShmTransmitter() {
    Disable();
}

/**
 * @tparam M: 发送的消息类型
 * @brief 设置Transmitter状态，并实例化segment_以及notifier_
 */
template <typename M>
void ShmTransmitter<M>::Enable() {
    if(this->enabled_) return;
    segment_ = SegmentFactory::CreateSegment(channel_id_);
    notifier_ = NotifierFactory::CreateNotifier();
    this->enabled_ = true;
}

/**
 * @tparam M: 发送的消息类型
 * @brief 设置Transmitter状态，并设置segment_以及notifier_为nullptr
 */
template <typename M>
void ShmTransmitter<M>::Disable() {
    if(this->enabled_) {
        segment_ = nullptr;
        notifier_ = nullptr;
        this->enabled = false;
    }
}

/**
 * @tparam M: 发送的消息类型
 * @param msg: 发送的消息
 * @param info： 发送的消息的信息（话题、帧号等）
 * @brief 发送消息
 */
template <typename M>
bool ShmTransmitter<M>::Transmit(const MessagePtr& msg, const MessageInfo& info) {
    return Transmit(*msg, info);
}

/**
 * @tparam M: 发送的消息类型
 * @param msg: 发送的消息
 * @param msg_info 发送的消息的信息（话题、帧号等）
 * @brief 发送消息
 */
template <typename M>
bool ShmTransmitter<M>::Transmit(const M& msg, const MessageInfo& msg_info) {
    if(!this->enabled_) {
        ADEBUG << "not enable."
        return false;
    }
    WritableBlock wb;
    // ADEBUG << "Debug Serialize start: " << Time::Now().ToMicrosecond();
    serialize::DataStream ds;
    ds << msg;
    std::size_t msg_size = ds.ByteSize();
    // ADEBUG << "Debug Serialize end: " << Time::Now().ToMicrosecond();
    
    if(!segment_->AcquireBlockToWrite(msg_size, &wb)) {
        AERROR << "acquire block failed."
        return false;
    }

    std::memcpy(wb.buf, ds.data(), msg_size);

    wb.block->set_msg_size(msg_size);
    char* msg_info_addr = reinterpret_cast<char*>(wb.buf) + msg_size;

    // 拷贝sender_id
    std::memcpy(msg_info_addr, msg_info.sender_id().data(), ID_SIZE);
    // 拷贝spare_id
    std::memcpy(msg_info_addr + ID_SIZE, msg_info.spaer_id().data(), ID_SIZE);
    // 拷贝seq
    *reinterpret_cast<uint64_t*>(msg_info_addr + ID_SIZE*2) = msg_info.seq_num();

    wb.block->set_msg_info_size(ID_SIZE*2 + sizeof(uint64_t));

    segment_->ReleaseWrittenBlock(wb);

    ReadableInfo readable_info(host_id_, wb.index, channel_id_);
    ADEBUG << "Writing shareedmem message: "
            << common::GlobalData::GetChannelById(channel_id_)
            << "to block: " << wb.index;
    return notifier_->Notify(readable_info);
}

} // transport
} // rcmw
} // hnu

#endif
