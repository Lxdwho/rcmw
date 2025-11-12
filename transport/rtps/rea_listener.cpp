/**
 * @brief 
 * @date 2025.11.12
 */

#include "rea_listener.h"
#include "rcmw/common/util.h"
using GUID_t = eprosima::fastrtps::rtps::GUID_t;

namespace hnu       {
namespace rcmw      {
namespace transport {

ReaListener::ReaListener(const NewMsgCallback& callback, const std::string& channel_name) 
    : callback_(callback), channel_name_(channel_name) {}

ReaListener::~ReaListener() {}

void ReaListener::onNewCacheChangeAdded(eprosima::fastrtps::rtps::RTPSReader* reader,
        const eprosima::fastrtps::rtps::CacheChange_t* const change) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto channel_id = common::Hash(channel_name_);

    char* ptr = reinterpret_cast<char*>(const_cast<GUID_t*>
                (&change->write_params.sample_identity().writer_guid()));

    Identity sender_id(false);
    sender_id.set_data(ptr);
    msg_info_.set_sender_id(sender_id);

    Identity spare_id(false);
    spare_id.set_data(ptr);
    msg_info_.set_spare_id(spare_id);

    uint64_t seq_num = ((uint64_t)change->sequenceNumber.high << 32) | change->sequenceNumber.low;
    msg_info_.set_seq_num(seq_num);

    std::shared_ptr<std::string> msg_str = 
        std::make_shared<std::string>((char*)change->serializedPayload.data, change->serializedPayload.length);
    
    callback_(channel_id, msg_str, msg_info_);

};

} // transport
} // rcmw
} // hnu
