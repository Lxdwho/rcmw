/**
 * @brief reader数据接收回调包装
 * @date 2025.12.16
 */

#include "rcmw/discovery/communication/reader_listener.h"
#include "rcmw/logger/log.h"

namespace hnu       {
namespace rcmw      {
namespace discovery {

ReaderListener::ReaderListener(const NewMsgCallback& callback) 
    : callback_(callback) {}

ReaderListener::~ReaderListener() {
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = nullptr;
}

void ReaderListener::onNewCacheChangeAdded(
            eprosima::fastrtps::rtps::RTPSReader* reader,
            const eprosima::fastrtps::rtps::CacheChange_t* const change) {
    if(callback_ == nullptr) {
        AINFO << "callback_ is nullptr";
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    std::shared_ptr<std::string> msg_str = 
        std::make_shared<std::string>((char*)change->serializedPayload.data,
            change->serializedPayload.length);
    callback_(*msg_str);
}

} // discovery
} // rcmw
} // hnu
