/**
 * @brief 基于rtps的数据分发器
 * @date 2025.12.11
 */

#ifndef _TRANSPORT_RTPS_DISPATCHER_H_
#define _TRANSPORT_RTPS_DISPATCHER_H_

#include "fastdds/rtps/rtps_fwd.h"
#include "rcmw/transport/rtps/rea_listener.h"
#include "dispatcher.h"
#include "rcmw/transport/rtps/participant.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

struct Reader {
    Reader() : reader(nullptr), reader_listener(nullptr) {}
    eprosima::fastrtps::rtps::RTPSReader* reader;
    eprosima::fastrtps::rtps::ReaderHistory* mp_history;
    ReaListenerPtr reader_listener;
};

class RtpsDispatcher;
using RtpsDispatcherPtr = RtpsDispatcher*;

class RtpsDispatcher : public Dispatcher {
public:
    virtual ~RtpsDispatcher();
    void Shutdown() override;

    template<typename M>
    void AddListener(const RoleAttributes& self_attr,
                     const MessageListener<M>& listener);
    
    template<typename M>
    void AddListener(const RoleAttributes& self_attr,
                     const RoleAttributes& opposite_attr,
                     const MessageListener<M>& listener);

    void set_participant(const ParticipantPtr& participant) {
        participant_ = participant;
    }
    
private:
    void OnMessage(uint64_t channel_id, 
                   const std::shared_ptr<std::string>& msg_str,
                   const MessageInfo& msg_info);
    void AddReader(const RoleAttributes& self_attr);
    std::unordered_map<uint64_t, Reader> readers_;
    std::mutex reader_mutex_;
    ParticipantPtr participant_;
    DECLARE_SINGLETON(RtpsDispatcher)
};

template<typename M>
void RtpsDispatcher::AddListener(const RoleAttributes& self_attr,
                    const MessageListener<M>& listener) {
    auto listener_adapter = [listener](const std::shared_ptr<std::string>& msg_str,
                                        const MessageInfo& msg_info) {
        auto msg = std::make_shared<M>();
        serialize::DataStream ds(*msg_str);
        ds >> *msg;
        listener(msg, msg_info);
    };
    Dispatcher::AddListener<std::string>(self_attr, listener_adapter);
    AddReader(self_attr);
}

template<typename M>
void RtpsDispatcher::AddListener(const RoleAttributes& self_attr,
                    const RoleAttributes& opposite_attr,
                    const MessageListener<M>& listener) {
    auto listener_adapter = [listener](const std::shared_ptr<std::string>& msg_str,
                                        const MessageInfo& msg_info) {
        auto msg = std::make_shared<M>();
        serialize::DataStream ds(*msg_str);
        ds >> *msg;
        listener(msg, msg_info);
    };
    Dispatcher::AddListener<std::string>(self_attr, opposite_attr, listener_adapter);
    AddReader(self_attr);
}

} // transport
} // rcmw
} // hnu

#endif
