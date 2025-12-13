/**
 * @brief 消息的信息：包含发送者id、备用ID、通道话题名称以及消息帧号
 * @date 2025.11.12
 */

#include "message_info.h"
#include "rcmw/logger/log.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

using namespace hnu::rcmw::logger;

const std::size_t MessageInfo::Ksize = 2 * ID_SIZE + sizeof(uint64_t);

MessageInfo::MessageInfo() : sender_id_(false), spare_id_(false) {}

MessageInfo::MessageInfo(const Identity& sender_id, uint64_t seq_num) :
    sender_id_(sender_id), seq_num_(seq_num), spare_id_(false) {}

MessageInfo::MessageInfo(const Identity& sender_id, 
    uint64_t seq_num, const Identity& spare_id) : 
    sender_id_(sender_id), seq_num_(seq_num), spare_id_(spare_id) {}

MessageInfo::MessageInfo(const MessageInfo& other) :
    sender_id_(other.sender_id_), channel_id_(other.channel_id_),
    seq_num_(other.seq_num_), spare_id_(other.spare_id_) {}

MessageInfo::~MessageInfo() {}

MessageInfo& MessageInfo::operator=(const MessageInfo& other) {
    if(this != &other) {
        sender_id_ = other.sender_id_;
        channel_id_ = other.channel_id_;
        seq_num_ = other.seq_num_;
        spare_id_ = other.spare_id_;
    }
    return *this;
}

bool MessageInfo::operator==(const MessageInfo& other) {
    return sender_id_ == other.sender_id_ &&
            channel_id_ == other.channel_id_ &&
            seq_num_ == other.seq_num_ &&
            spare_id_ == other.spare_id_;
}

bool MessageInfo::operator!=(const MessageInfo& other) {
    return !(*this == other);
}


/* 序列化操作 */
bool MessageInfo::SerializeTo(std::string* dst) const {
    if(dst == nullptr) return false;
    dst->assign(sender_id_.data(), ID_SIZE);
    dst->append(reinterpret_cast<const char*>(seq_num_), sizeof(seq_num_));
    dst->append(spare_id_.data(), ID_SIZE);
    return true;
}

bool MessageInfo::SerializeTo(char* dst, std::size_t len) const {
    if(dst == nullptr || len < Ksize) return false;
    char* ptr = dst;
    std::memcpy(ptr, sender_id_.data(), ID_SIZE);
    ptr += ID_SIZE;
    std::memcpy(ptr, reinterpret_cast<const char*>(seq_num_), sizeof(seq_num_));
    ptr += sizeof(seq_num_);
    std::memcpy(ptr, spare_id_.data(), ID_SIZE);
    return true;
}

bool MessageInfo::DeserializeFrom(const std::string& src) {
    return DeserializeFrom(src.data(), src.size());
}

bool MessageInfo::DeserializeFrom(const char* src, std::size_t len) {
    if(src == nullptr) return false;
    if(len != Ksize) {
        AERROR << "src size mismatch, given[" << len << "] target[" << Ksize << "]";
        return false;
    }

    char* ptr = const_cast<char*>(src);
    sender_id_.set_data(ptr);
    ptr += ID_SIZE;
    std::memcpy(reinterpret_cast<char*>(&seq_num_), ptr, sizeof(seq_num_));
    ptr += sizeof(seq_num_);
    spare_id_.set_data(ptr);
    return true;
}

} // transport
} // rcmw
} // hnu
