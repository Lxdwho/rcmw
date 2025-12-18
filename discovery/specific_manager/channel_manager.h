/**
 * @brief 话题拓扑管理
 * @date 2025.12.18
 */

#ifndef _DISCOVERY_CHANNEL_MANEGER_H_
#define _DISCOVERY_CHANNEL_MANEGER_H_

#include "rcmw/discovery/specific_manager/manager.h"
#include "rcmw/discovery/container/graph.h"
#include "rcmw/discovery/container/single_value_warehouse.h"
#include "rcmw/discovery/container/multi_value_warehouse.h"
#include "rcmw/discovery/role/role.h"
#include <unordered_set>
#include <vector>

namespace hnu       {
namespace rcmw      {
namespace discovery {

class TopologyManager;

class ChannelManager : public Manager {
    friend class TopologyManager;
public:
    using RoleAttrVec = std::vector<RoleAttributes>;
    using WriterWarehouse = MutilValueWarehouse;
    using ReaderWarehouse = MutilValueWarehouse;
    using ExemptedMessageTypes = std::unordered_set<std::string>;

    ChannelManager();
    virtual ~ChannelManager();

    void GetWritersOfChannel(const std::string& channel_name, RoleAttrVec* writers);
    void GetReadersOfChannel(const std::string& channel_name, RoleAttrVec* readers);

    void GetWritersOfNode(const std::string& node_name, RoleAttrVec* writers);
    void GetReadersOfNode(const std::string& node_name, RoleAttrVec* readers);

    void GetUpstreamOfNode(const std::string& node_name, RoleAttrVec* upstream_nodes);
    void GetDownStreamOfnode(const std::string& node_name, RoleAttrVec* downstream_nodes);

    FlowDirection GetFlowDirection(const std::string& lhs_node_name, const std::string& rhs_node_name);

    void GetChannelNames(std::vector<std::string>* channels);
    void GetMsgType(const std::string& channel_name, std::string* msg_type);

    bool HasWriter(const std::string& channel_name);
    void GetWriters(RoleAttrVec* writers);

    bool HasReader(const std::string& channel_name);
    void GetReaders(RoleAttrVec* readers);

    bool IsMessageTypeMatching(const std::string& lhs, const std::string& rhs);

private:
    bool Check(const RoleAttributes& attr) override;
    void Dispose(const ChangeMsg& msg) override;
    void OnTopoModuleLeave(const std::string& host_name, int process_id) override;
    void DisposeJoin(const ChangeMsg& msg);
    void DisposeLeave(const ChangeMsg& msg);
    void ScanMessageType(const ChangeMsg& msg);
    
    ExemptedMessageTypes exempted_msg_types_;
    Graph node_graph_;

    WriterWarehouse node_writers_;
    WriterWarehouse node_readers_;

    WriterWarehouse channel_writers_;
    ReaderWarehouse channel_readers_;
};

} // discovery
} // rcmw
} // hnu

#endif
