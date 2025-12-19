/**
 * @brief 拓扑集成管理
 * @date 2025.12.19
 */

#include "rcmw/discovery/topology_manager.h"
#include "rcmw/common/global_data.h"
#include "rcmw/logger/log.h"
#include "rcmw/time/time.h"

namespace hnu       {
namespace rcmw      {
namespace discovery {

using eprosima::fastrtps::rtps::ParticipantDiscoveryInfo;

TopologyManager::TopologyManager() :
        init_(false), 
        node_manager_(nullptr), 
        channel_manager_(nullptr),
        participant_(nullptr),
        participant_listener_(nullptr) {
    Init();
}

TopologyManager::~TopologyManager() { Shutdown(); }
void TopologyManager::Shutdown() {
    if(!init_.exchange(false)) return;

    node_manager_->Shutdown();
    channel_manager_->Shutdown();

    delete participant_listener_;
    participant_listener_ = nullptr;

    change_signal_.DisconnectAllSlots();
}

TopologyManager::ChangeConnection TopologyManager::AddChangeListener(
                                const ChangeFunc& func) {
    return change_signal_.Connect(func);
}

void TopologyManager::RemoveChangeListener(
            const TopologyManager::ChangeConnection& conn) {
    change_signal_.DisConnect(conn);
}

bool TopologyManager::Init() {
    if(init_.exchange(true)) return true;

    node_manager_ = std::make_shared<NodeManager>();
    channel_manager_ = std::make_shared<ChannelManager>();

    CreateParticipant();

    bool result = InitNodeManager() && InitChannelManager();

    if(!result) {
        AINFO << "init manager failed.";
        participant_ = nullptr;
        delete participant_listener_;
        participant_listener_ = nullptr;
        node_manager_ = nullptr;
        channel_manager_ = nullptr;
        init_.store(false);
        return false;
    }
    return true;
}

bool TopologyManager::InitNodeManager() {
    return node_manager_->StartDiscovery(
            participant_->fastrtps_participant());
}

bool TopologyManager::InitChannelManager() {
    return channel_manager_->StartDiscovery(
            participant_->fastrtps_participant());
}

bool TopologyManager::CreateParticipant() {
    std::string participant_name = 
        common::GlobalData::Instance()->HostName() + '+' +
        std::to_string(common::GlobalData::Instance()->ProcessId());
    AINFO << "participant name: " << participant_name;

    participant_listener_ = new ParticipantListener(std::bind(
        &TopologyManager::OnparticipantChange, this, std::placeholders::_1
    ));

    participant_ = std::make_shared<transport::Participant>(
        participant_name, 11511, participant_listener_);
    return true;
}

void TopologyManager::OnparticipantChange(const PartInfo& info) {
    ChangeMsg msg;
    if(!Convert(info, &msg)) return;
    if(!init_.load()) return;
    if(msg.operate_type == OperateType::OPT_LEAVE) {
        auto& host_name = msg.role_attr.host_name;
        int process_id = msg.role_attr.process_id;
        node_manager_->OnTopoModuleLeave(host_name, process_id);
        channel_manager_->OnTopoModuleLeave(host_name, process_id);
    }
    change_signal_(msg);
}

bool TopologyManager::Convert(const PartInfo& info, ChangeMsg* change_msg) {
    auto guid = info.info.m_guid;
    auto status = info.status;
    std::string participant_name("");
    OperateType opt_type = OperateType::OPT_JOIN;

    switch (status)
    {
    case ParticipantDiscoveryInfo::DISCOVERY_STATUS::DISCOVERED_PARTICIPANT:
        participant_name = info.info.m_participantName;
        participant_names_[guid] = participant_name;
        opt_type = OperateType::OPT_JOIN;
        break;

    case ParticipantDiscoveryInfo::DISCOVERY_STATUS::REMOVED_PARTICIPANT:

    case ParticipantDiscoveryInfo::DISCOVERY_STATUS::DROPPED_PARTICIPANT:
        if(participant_names_.find(guid) != participant_names_.end()) {
            participant_name = participant_names_[guid];
            participant_names_.erase(guid);
        }
        opt_type = OperateType::OPT_LEAVE;
        break;
    
    default:
        break;
    }

    std::string host_name("");
    int process_id = 0;
    if(!ParseParticipantName(participant_name, &host_name, &process_id))
        return false;

    change_msg->timestamp = Time::Now().ToNanosecond();
    change_msg->change_type = ChangeType::CHANGE_PARTICIPANT;
    change_msg->operate_type = opt_type;
    change_msg->role_type = RoleType::ROLE_PARTICIPANT;

    change_msg->role_attr.host_name = host_name;
    change_msg->role_attr.process_id = process_id;
    return true;
}

bool TopologyManager::ParseParticipantName(const std::string& participant_name, 
                            std::string* host_name, int* process_id) {
    auto pos = participant_name.find('+');
    if(pos == std::string::npos) {
        AINFO << "participant_name [" << participant_name << "] format mismatch.";
        return false;
    }
    
    *host_name = participant_name.substr(0, pos);
    std::string pid_str = participant_name.substr(pos + 1);

    try {
        *process_id = std::stoi(pid_str);
    }
    catch (const std::exception& e) {
        AINFO << "invalid process_id: " << e.what();
        return false;
    }
    return true;
}

} // discovery
} // rcmw
} // hnu
