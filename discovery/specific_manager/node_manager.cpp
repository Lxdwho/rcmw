/**
 * @brief node拓扑管理
 * @date 2025.12.18
 */

#include "rcmw/discovery/specific_manager/node_manager.h"
#include "rcmw/logger/log.h"
#include "rcmw/common/global_data.h"

namespace hnu       {
namespace rcmw      {
namespace discovery {

NodeManager::NodeManager() {
    allowed_role_ |= 1 << RoleType::ROLE_NODE;
    change_type_ = ChangeType::CHANGE_NODE;
    channel_name_ = "node_change_broadcast";
}

NodeManager::~NodeManager() {}

bool NodeManager::HasNode(const std::string& node_name) {
    uint64_t key = common::GlobalData::RegisterNode(node_name);
    return nodes_.Search(key);
}

void NodeManager::GetNodes(RoleAttrVec* nodes) {
    RETURN_IF_NULL(nodes);
    nodes_.GetAllRoles(nodes);
}

bool NodeManager::Check(const RoleAttributes& attr) {
    RETURN_VAL_IF(attr.node_name.empty(), false);
    RETURN_VAL_IF(!attr.node_id, false);
    return true;
}

void NodeManager::Dispose(const ChangeMsg& msg) {
    if(msg.operate_type == OperateType::OPT_JOIN) DisposeJoin(msg);
    else DisposeLeave(msg);
    Notify(msg);
}

void NodeManager::OnTopoModuleLeave(const std::string& host_name, 
                                    int process_id) {
    RETURN_IF(!is_discovery_started_.load());

    RoleAttributes attr;
    attr.host_name = host_name;
    attr.process_id = process_id;
    std::vector<RolePtr> nodes_to_remove;
    nodes_.Search(attr, &nodes_to_remove);
    for(auto& node : nodes_to_remove) nodes_.Remove(node->attributes().node_id);

    ChangeMsg msg;
    for(auto& node : nodes_to_remove) {
        Convert(node->attributes(), RoleType::ROLE_NODE, 
                    OperateType::OPT_LEAVE, &msg);
        Notify(msg);
    }
}

void NodeManager::DisposeJoin(const ChangeMsg& msg) {
    auto node = std::make_shared<RoleNode>(msg.role_attr, msg.timestamp);
    uint64_t key = node->attributes().node_id;
    if(!nodes_.Add(key, node, false)) {
        RolePtr existing_node;
        if(!nodes_.Search(key, &existing_node)) {
            nodes_.Add(key, node);
            return;
        }

        RolePtr newer_node = existing_node;
        if(node->IsEarlierThan(*newer_node)) nodes_.Add(key, node);
        else newer_node = node;

        if(newer_node->attributes().process_id == process_id_ &&
           newer_node->attributes().host_name == host_name_) {
            AINFO << "this process will be terminated due to duplicated node["
                  << node->attributes().node_name
                  << "], please ensure that each node has a unique name.";
        }
    }
}

void NodeManager::DisposeLeave(const ChangeMsg& msg) {
    auto node = std::make_shared<RoleNode>(msg.role_attr, msg.timestamp);
    nodes_.Remove(node->attributes().node_id);
}

} // discovery
} // rcmw
} // hnu
