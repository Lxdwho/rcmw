/**
 * @brief 基于rtps的数据分发器
 * @date 2025.12.11
 */

#include "rtps_dispatcher.h"
#include "rcmw/transport/rtps/attributes_filler.h"
#include "fastrtps/rtps/RTPSDomain.h"
#include "fastrtps/rtps/reader/RTPSReader.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

RtpsDispatcher::RtpsDispatcher() : participant_(nullptr) {}
RtpsDispatcher::~RtpsDispatcher() { Shutdown(); }

void RtpsDispatcher::Shutdown() {
    if(is_shutdown_.exchange(true)) return;

    {
        std::lock_guard<std::mutex> lock(reader_mutex_);
        for(auto& item : readers_) item.second.reader = nullptr;
    }
    participant_ = nullptr;
}

void RtpsDispatcher::OnMessage(uint64_t channel_id, 
                const std::shared_ptr<std::string>& msg_str,
                const MessageInfo& msg_info) {
    if(is_shutdown_.load()) return;
    ListenerHandlerBasePtr* handler_base = nullptr;
    if(msg_listeners_.Get(channel_id, &handler_base)) {
        auto handler = std::dynamic_pointer_cast<ListenerHandler<std::string>>(*handler_base);
        handler->Run(msg_str, msg_info);
    }
}

void RtpsDispatcher::AddReader(const RoleAttributes& self_attr) {
    if(participant_ == nullptr) {
        ADEBUG << "please set participant firstly.";
        return;
    }
    uint64_t channel_id = self_attr.channel_id;
    std::lock_guard<std::mutex> lock(reader_mutex_);
    if(readers_.count(channel_id) > 0) return;

    Reader new_reader;
    RtpsReaderAttributes reader_attr;
    auto& qos = self_attr.qos_profile;
    
    AttributesFiller::FillInReaderAttr(self_attr.channel_name, qos, &reader_attr);
    
    new_reader.reader_listener = std::make_shared<ReaListener>(std::bind(
                    &RtpsDispatcher::OnMessage, this, std::placeholders::_1, 
                    std::placeholders::_2, std::placeholders::_3), 
                    self_attr.channel_name);
    
    new_reader.mp_history = new ReaderHistory(reader_attr.hatt);
    
    new_reader.reader = RTPSDomain::createRTPSReader(
                        participant_->fastrtps_participant(), 
                        reader_attr.ratt, new_reader.mp_history, 
                        new_reader.reader_listener.get());

    bool reg = participant_->fastrtps_participant()->registerReader(
                            new_reader.reader, reader_attr.tatt, reader_attr.rqos);
    readers_[channel_id] = new_reader;
}

} // transport
} // rcmw
} // hnu
