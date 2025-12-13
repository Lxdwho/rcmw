/**
 * @brief 消息的信息：包含发送者id、备用ID、通道话题名称以及消息帧号
 * @date 2025.11.12
 */

#ifndef _MESSAGE_INFO_H_
#define _MESSAGE_INFO_H_

#include "rcmw/transport/common/identity.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

class MessageInfo {
public:
    MessageInfo();
    MessageInfo(const Identity& sender_id, uint64_t seq_num);
    MessageInfo(const Identity& sender_id, uint64_t seq_num,
                const Identity& spare_id);
    MessageInfo(const MessageInfo& other);
    virtual ~MessageInfo();

    MessageInfo& operator=(const MessageInfo& other);
    bool operator==(const MessageInfo& other);
    bool operator!=(const MessageInfo& other);

    /* 序列化操作 */
    bool SerializeTo(std::string* dst) const;
    bool SerializeTo(char* dst, std::size_t len) const;
    bool DeserializeFrom(const std::string& src);
    bool DeserializeFrom(const char* src, std::size_t len);

    /* 变量操作 */
    const Identity& sender_id() const { return sender_id_; }
    void set_sender_id(const Identity& sender_id) { sender_id_ = sender_id; }
    
    uint64_t channel_id() const { return channel_id_; }
    void set_channel_id(uint64_t channel_id) { channel_id_ = channel_id; }

    uint64_t seq_num() const { return seq_num_; }
    void set_seq_num(uint64_t seq_num) { seq_num_ = seq_num; }

    const Identity& spaer_id() const { return spare_id_; }
    void set_spare_id(const Identity& spaer_id) { spare_id_ = spaer_id; }

    static const std::size_t Ksize;
private:
    Identity sender_id_;        // 发送者ID
    Identity spare_id_;         // 备用ID
    uint64_t channel_id_ = 0;   // 通道/话题ID？
    uint64_t seq_num_ = 0;      // 消息帧号
};

}
}
}

#endif
