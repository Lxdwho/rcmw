/**
 * @brief 拓扑管理类
 * @date 2025.12.17
 */

#include "rcmw/discovery/specific_manager/manager.h"
#include "rcmw/common/global_data.h"
#include "rcmw/logger/log.h"
#include "rcmw/transport/rtps/attributes_filler.h"
#include "rcmw/transport/qos/qos_profile_conf.h"
#include "rcmw/transport/rtps/participant.h"
#include "rcmw/time/time.h"


namespace hnu       {
namespace rcmw      {
namespace discovery {

using namespace hnu::rcmw::transport;

Manager::Manager() : 
        is_shutdown_(false), 
        is_discovery_started_(false), 
        allowed_role_(0), 
        change_type_(ChangeType::CHANGE_PARTICIPANT), 
        channel_name_(""), 
        writer_(nullptr), 
        reader_(nullptr), 
        listener_(nullptr) {
    host_name_ = common::GlobalData::Instance()->HostName();
    process_id_ = common::GlobalData::Instance()->ProcessId();
}

Manager::~Manager() { Shutdown(); }
void Manager::Shutdown() {
    if(is_shutdown_.exchange(true)) return;
    StopDiscovery();
    signal_.DisconnectAllSlots();
}

bool Manager::StartDiscovery(RtpsParticipant* participant) {
    if(participant == nullptr) return false;
    if(is_discovery_started_.exchange(true)) return true;
    if(!CreateWriter(participant) || !CreateReader(participant)) {
        AERROR << "create writer or reader failed.";
        StopDiscovery();
        return false;
    }
    return true;
}

void Manager::StopDiscovery() {
    if(!is_discovery_started_.exchange(false)) return;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if(writer_ != nullptr)  // 不防止出现野指针？
            eprosima::fastrtps::rtps::RTPSDomain::removeRTPSWriter(writer_);
    }

    if(reader_ != nullptr)  // 不防止出现野指针？
        eprosima::fastrtps::rtps::RTPSDomain::removeRTPSReader(reader_);

    if(listener_ != nullptr) {
        delete listener_;
        listener_ = nullptr;
    }
}

bool Manager::Join(const RoleAttributes& attr, RoleType role, bool need_write) {
    if(is_shutdown_.load()) {
        ADEBUG << "the manager has been shutdown.";
        return false;
    }
    
    RETURN_VAL_IF(!((1 << role) & allowed_role_), false);
    RETURN_VAL_IF(!Check(attr), false);

    ChangeMsg msg;
    Convert(attr, role, OperateType::OPT_JOIN, &msg);

    Dispose(msg);
    if(need_write) return Write(msg);
    return true;
}

bool Manager::Leave(const RoleAttributes& attr, RoleType role) {
    if(is_shutdown_.load()) {
        ADEBUG << "the manager has been shutdown.";
        return false;
    }

    RETURN_VAL_IF(!((1 << role) & allowed_role_), false);
    RETURN_VAL_IF(!Check(attr), false);
    
    ChangeMsg msg;
    Convert(attr, role, OperateType::OPT_LEAVE, &msg);
    Dispose(msg);
    if(NeedPublish(msg)) return Write(msg);
    return true;
}

Manager::ChangeConnection Manager::AddChangeListener(const ChangeFunc& func) {
    return signal_.Connect(func);
}

void Manager::RemoveChangeListener(const ChangeConnection& conn) {
    auto local_conn = conn;
    local_conn.Disconnect();
}

bool Manager::CreateWriter(RtpsParticipant* participant) {
    RtpsWriterAttributes writer_attr;
    AttributesFiller::FillInWriterAttr(channel_name_, 
            QosProfileConf::QOS_PROFILE_TOPO_CHANGE, &writer_attr);
    
    writer_history_ = new WriterHistory(writer_attr.hatt);
    writer_ = RTPSDomain::createRTPSWriter(participant, writer_attr.watt, writer_history_);
    
    bool ret = participant->registerWriter(writer_, writer_attr.tatt, writer_attr.wqos);
    return ret;
}

bool Manager::CreateReader(RtpsParticipant* participant) {
    RtpsReaderAttributes reader_attr;
    AttributesFiller::FillInReaderAttr(channel_name_, 
                                       QosProfileConf::QOS_PROFILE_TOPO_CHANGE, 
                                       &reader_attr);
    listener_ = new ReaderListener(std::bind(&Manager::OnRemoteChange, this,
                    std::placeholders::_1));
    eprosima::fastrtps::rtps::ReaderHistory* mp_history = 
                    new ReaderHistory(reader_attr.hatt);
    reader_ = RTPSDomain::createRTPSReader(participant, reader_attr.ratt, mp_history, listener_);
    bool ret = participant->registerReader(reader_, reader_attr.tatt, reader_attr.rqos);
    return ret;
}

bool Manager::NeedPublish(const ChangeMsg& msg) const {
    (void)msg;
    return true;
}

void Manager::Notify(const ChangeMsg& msg) { signal_(msg); }

bool Manager::Write(const ChangeMsg& msg) {
    if(!is_discovery_started_.load()) {
        ADEBUG << "discovery is not started.";
        return false;
    }

    serialize::DataStream ds;
    ds << msg;

    { 
        std::lock_guard<std::mutex> lock(mutex_);
        if(writer_ != nullptr) {
            CacheChange_t* ch = writer_->new_change([]()->uint32_t {
                return 255;
            }, ALIVE);
            ch->serializedPayload.length = ds.size();
            std::memcpy((char*)ch->serializedPayload.data, ds.data(), ds.size());
            bool flag = writer_history_->add_change(ch);
            if(!flag) {
                writer_->remove_older_changes(20);
                writer_history_->add_change(ch);
            }
        }
    }
    return true;
}

void Manager::OnRemoteChange(const std::string& msg_str) {
    if(is_shutdown_.load()) {
        ADEBUG << "the maneger has been shutdown.";
        return;
    }
    
    ChangeMsg msg;
    serialize::DataStream ds(msg_str);
    ds >> msg;

    if(IsFromSameProcess(msg)) {
        ADEBUG << "FromSameProcess";
        return;
    }
    RETURN_IF(!Check(msg.role_attr));
    Dispose(msg);
}

bool Manager::IsFromSameProcess(const ChangeMsg& msg) {
    auto& host_name = msg.role_attr.host_name;
    int process_id = msg.role_attr.process_id;
    if(process_id != process_id_ || host_name != host_name_) return false;
    return true;
}

void Manager::Convert(const RoleAttributes& attr, RoleType role, 
                        OperateType opt, ChangeMsg* msg) {
    msg->timestamp = rcmw::Time::Now().ToNanosecond();
    msg->change_type = change_type_;
    msg->operate_type = opt;
    msg->role_type = role;
    msg->role_attr = attr;

    if(msg->role_attr.host_name.empty())
        msg->role_attr.host_name = host_name_;
    if(!msg->role_attr.process_id)
        msg->role_attr.process_id = process_id_;
}

} // discovery
} // rcmw
} // hnu
