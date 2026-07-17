/**
 * @brief rcmw初始化
 * @date 2025.12.27
 */

#include "init.h"
#include "common/global_data.h"
#include "scheduler/scheduler_factory.h"
#include "transport/transport.h"
#include "discovery/topology_manager.h"
#include "state.h"
#include <string>
#include <cstdlib>

namespace hnu   {
namespace rcmw  {

bool Init(const char* binary_name) {
    std::string logfile_name = (std::string)binary_name + ".log";
    Logger_Init(logfile_name);

    [[maybe_unused]] auto global_data = common::GlobalData::Instance();

    std::atexit([]{ 
        Clear();  // 程序退出时自动执行
    });
    SetState(STATE_INITIALIZED);
    return true;
}

void Clear() {
    SetState(STATE_SHUTTING_DOWN);
    scheduler::CleanUp();
    transport::Transport::CleanUp();
    discovery::TopologyManager::CleanUp();
    SetState(STATE_SHUTDOWN);
    AINFO << "all cleared";
}

std::unique_ptr<Node> CreateNode(const std::string& node_name, 
                                 const std::string& name_space) {
    return std::unique_ptr<Node>(new Node(node_name, name_space));
}

} // rcmw
} // hnu
