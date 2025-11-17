/**
 * @brief 重看、重写
 * @date 2025.11.14
 */

#include "global_data.h"

namespace hnu    {
namespace rcmw   {
namespace common {

/* 静态成员变量，类内声明，类外初始化 */
AtomicHashMap<uint64_t, std::string, 256> GlobalData::channel_id_map_;
AtomicHashMap<uint64_t, std::string, 512> GlobalData::node_id_map_;
AtomicHashMap<uint64_t, std::string, 256> GlobalData::task_id_map_;

namespace {
const std::string& KEmptyString = "";
std::string program_path() {
    char path[4096];
    auto len = readlink
}
}

GlobalData::GlobalData() { }

GlobalData::~GlobalData() { }

int GlobalData::ProcessId() const { }

void GlobalData::SetProcessGroup(const std::string& process_group) { }
const std::string& GlobalData::ProcessGroup() const { }

void GlobalData::SetComponentNums(const int component_nums) { }
int GlobalData::ComponentNums() const { }

const std::string & GlobalData::HostIp() const { }
const std::string & GlobalData::HostName() const { }

const RcmwConfig& GlobalData::Config() const { }


std::string GlobalData::GetChannelById(uint64_t id) { }
uint64_t GlobalData::RegisterChannel(const std::string& channel) { }
uint64_t GlobalData::RegisterTaskName(const std::string& task_name) { }
std::string GlobalData::GetTaskNameById(uint64_t id) { }

void GlobalData::InitHostInfo() { }
bool GlobalData::InitConfig() { }



} // common
} // rcmw
} // hnu
