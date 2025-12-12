/**
 * @brief 基于rtps的Transmitter
 * @date 2025.11.17
 */

#ifndef _RTPS_TRANSMITTER_H_
#define _RTPS_TRANSMITTER_H_

#include <memory>
#include <string>
#include "transmitter.h"
#include "rcmw/config/RoleAttributes.h"
#include "rcmw/transport/rtps/participant.h"
#include "rcmw/transport/rtps/attributes_filler.h"
#include "fastrtps/rtps/RTPSDomain.h"
#include "rcmw/serialize/data_stream.h"
#include "rcmw/logger/log.h"
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps;

namespace hnu       {
namespace rcmw      {
namespace transport {

using namespace config;

template<typename M>
class RtpsTransmitter : public Transmitter<M> {
public:
    using MessagePtr = std::shared_ptr<M>;
    RtpsTransmitter(const RoleAttributes& attr, 
                    const ParticipantPtr& participent);
    virtual ~RtpsTransmitter();
    void Enable() override;
    void Disable() override;
    bool Transmit(const MessagePtr& msg, const MessageInfo& msg_info);
private:
    bool Transmit(const M& msg, const MessageInfo& msg_info);
    ParticipantPtr participant_;
    eprosima::fastrtps::rtps::RTPSWriter* rtps_write;
    eprosima::fastrtps::rtps::WriterHistory* mp_history;
};

template<typename M>
RtpsTransmitter<M>::RtpsTransmitter(const RoleAttributes& attr, 
                const ParticipantPtr& participent) :
    Transmitter<M>(attr), 
    participant_(participent), 
    rtps_write(nullptr) {}

template<typename M>
RtpsTransmitter<M>::~RtpsTransmitter() {
    Disable();
}

template<typename M>
void RtpsTransmitter<M>::Enable() {
    if(this->enabled_) return;
    /* 创建 RTPSWrite 配置信息实例 */
    RtpsWriterAttributes writer_attr;
    /* 填充RtpsWrite */
    AttributesFiller::FillInWriterAttr(this->attr_.channel_name,
        this->attr_.qos_profile, &writer_attr);
    /* 创建Rtps WriterHistory */
    mp_history = new WriterHistory(writer_attr.hatt);

    /* 创建rtps write */
    rtps_write = RTPSDomain::createRTPSWriter(
        participant_->fastrtps_participant(), 
        writer_attr.watt, mp_history);
    
    /* 注册rtps write */
    ADEBUG << "I'm OK!";
    bool reg = participant_->fastrtps_participant()->registerWriter(
        rtps_write, writer_attr.tatt, writer_attr.wqos);
    ADEBUG << "I'm OK1!";
    if(reg) this->enabled_ = true;
}

template<typename M>
void RtpsTransmitter<M>::Disable() {
    if(this->enabled_) {
        rtps_write = nullptr;
        this->enabled_ = false;
    }
}

template<typename M>
bool RtpsTransmitter<M>::Transmit(const MessagePtr& msg,
        const MessageInfo& msg_info) {
    return Transmit(*msg, msg_info);
}

template<typename M>
bool RtpsTransmitter<M>::Transmit(const M& msg, 
        const MessageInfo& msg_info) {
    if(!this->enabled_) {
        ADEBUG << "not enable.";
        return false;
    }

    CacheChange_t* ch = rtps_write->new_change([]()->uint32_t {
        return 255;
    }, ALIVE);

    eprosima::fastrtps::rtps::WriteParams wparams;
    char* ptr = reinterpret_cast<char*>
                (&wparams.related_sample_identity().writer_guid());
    memcpy(ptr, msg_info.sender_id().data(), ID_SIZE);
    memcpy(ptr + ID_SIZE, msg_info.spaer_id().data(), ID_SIZE);

    wparams.related_sample_identity().sequence_number().high = 
        (int32_t)((msg_info.seq_num() & 0xFFFFFFFF00000000) >> 32);
    wparams.related_sample_identity().sequence_number().low = 
        (int32_t)(msg_info.seq_num() & 0xFFFFFFFF);
    
    serialize::DataStream ds;
    ds << msg;

    ch->serializedPayload.length = ds.size();
    std::memcpy((char*)ch->serializedPayload.data, ds.data(), ds.size());

    bool flag = mp_history->add_change(ch, wparams);

    if(!flag) {
        rtps_write->remove_older_changes(20);
        mp_history->add_change(ch, wparams);
    }
    if(participant_->is_shutdown()) return false;
    return true;
}

} // transport
} // rcmw
} // hnu

#endif
