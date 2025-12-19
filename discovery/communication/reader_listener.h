/**
 * @brief reader数据接收回调包装
 * @date 2025.12.16
 */

#ifndef _DISCOVERY_READER_LISTENER_H_
#define _DISCOVERY_READER_LISTENER_H_

#include "fastrtps/rtps/reader/ReaderListener.h"
#include "fastrtps/rtps/rtps_fwd.h"
#include "rcmw/transport/message/message_info.h"
#include <mutex>
#include <functional>
#include <memory>

namespace hnu       {
namespace rcmw      {
namespace discovery {

class ReaderListener : public eprosima::fastrtps::rtps::ReaderListener {
public:
    using NewMsgCallback = std::function<void(const std::string&)>;
    explicit ReaderListener(const NewMsgCallback& callback);
    virtual ~ReaderListener();

    void onNewCacheChangeAdded(eprosima::fastrtps::rtps::RTPSReader* reader,
                const eprosima::fastrtps::rtps::CacheChange_t* const change) override;

    void onReaderMatched(eprosima::fastrtps::rtps::RTPSReader* reader, 
                         eprosima::fastrtps::rtps::MatchingInfo& info) override {}
private:
    NewMsgCallback callback_;
    transport::MessageInfo msg_info_;
    std::mutex mutex_;
};

} // discovery
} // rcmw
} // hnu

#endif
