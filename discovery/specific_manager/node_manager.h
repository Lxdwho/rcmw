/**
 * @brief node拓扑管理
 * @date 2025.12.18
 */

#ifndef _DISCOVERY_NODE_MANAGER_H_
#define _DISCOVERY_NODE_MANAGER_H_

#include "rcmw/discovery/specific_manager/manager.h"
#include "rcmw/discovery/container/single_value_warehouse.h"
#include "rcmw/discovery/role/role.h"
#include <memory>
#include <string>
#include <vector>

namespace hnu       {
namespace rcmw      {
namespace discovery {

class TopologyManager;

class NodeManager : public Manager {
    friend class TopologyManager;
public:
    using RoleAttrVec = std::vector<RoleAttributes>;
    using NodeWarehouse = SingleValueWarehouse;

    NodeManager();
    virtual ~NodeManager();

    bool HasNode(const std::string& node_name);
    void GetNodes(RoleAttrVec* nodes);


private:
    bool Check(const RoleAttributes& attr) override;
    void Dispose(const ChangeMsg& msg) override;
    void OnTopoModuleLeave(const std::string& host_name, int process_id) override;

    void DisposeJoin(const ChangeMsg& msg);
    void DisposeLeave(const ChangeMsg& msg);

    NodeWarehouse nodes_;
};

} // discovery
} // rcmw
} // hnu

#endif
