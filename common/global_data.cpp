/**
 * @brief 重看、重写
 * @date 2025.11.14
 */

#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include "global_data.h"
#include "rcmw/common/file.h"
#include "rcmw/common/util.h"
#include "rcmw/logger/log.h"
#include "rcmw/config/conf_parse.h"
#include "rcmw/common/environment.h"

namespace hnu    {
namespace rcmw   {
namespace common {

/* 静态成员变量，类内声明，类外初始化 */
AtomicHashMap<uint64_t, std::string, 256> GlobalData::channel_id_map_;
AtomicHashMap<uint64_t, std::string, 512> GlobalData::node_id_map_;
AtomicHashMap<uint64_t, std::string, 256> GlobalData::task_id_map_;

namespace {
    // 返回当前进程执行路径
    const std::string KEmptyString = "";
    std::string program_path() {
        char path[4096];
        auto len = readlink("proc/self/exe", path, sizeof(path) - 1);
        if(len == -1) return KEmptyString;
        path[len] = '\0';
        return std::string(path);
    }
}

GlobalData::GlobalData() { 
    InitHostInfo();
    InitConfig();

    porcess_id_ = getpid();
    
    auto prog_path = program_path();
    if(!prog_path.empty()) 
        process_group_ = GetFileName(prog_path) + '_' + std::to_string(porcess_id_);
    else
        process_group_ = "rcmw_default_" + std::to_string(porcess_id_);
}

GlobalData::~GlobalData() { }

/* 返回进程ID */
int GlobalData::ProcessId() const { return porcess_id_; }

/* 设置进程组 */
void GlobalData::SetProcessGroup(const std::string& process_group) { 
    process_group_ = process_group_;
}
/* 返回进程组 */
const std::string& GlobalData::ProcessGroup() const { return process_group_; }

/* 设置组件编号 */
void GlobalData::SetComponentNums(const int component_nums) { 
    component_nums_ = component_nums; 
}
int GlobalData::ComponentNums() const { return component_nums_; }

/* 返回主机IP */
const std::string & GlobalData::HostIp() const { return host_ip_; }

/* 返回主机名称 */
const std::string & GlobalData::HostName() const { return host_name_; }

/* 配置文件操作 */
const RcmwConfig& GlobalData::Config() const { return config_; }
bool GlobalData::InitConfig() {
    std::string config_path("rcmw/conf/rcmw.pb.conf");
    config_path = GetAbsolutePath(WorkRoot(), "conf/rcmw.pb.conf");
    if(!config::GetRcmwConfFromFile(config_path, &config_)) {
        AERROR << "Read rcmw/conf/rcmw.pb.conf form absolute path failed!";
        return false;
    }
    AINFO << "Read rcmw/conf/rcmw.pb.conf form absolute path success!";
    return true;
}

/* 获取channel名，通过id */
std::string GlobalData::GetChannelById(uint64_t id) {
    std::string* channel = nullptr;
    if(channel_id_map_.Get(id, &channel)) return *channel;
    return KEmptyString;
}

/* 注册channel，返回id */
uint64_t GlobalData::RegisterChannel(const std::string& channel) {
    auto id = Hash(channel);
    while(channel_id_map_.Has(id)) {
        std::string* name = nullptr;
        channel_id_map_.Get(id, &name);
        if(channel == *name) break;
        ++id;
        AWARN << "Channel name hash collision: " << channel << "<=>" << *name;
    }
    channel_id_map_.Set(id, channel);
    return id;
}

/* 注册Node，返回id */
uint64_t GlobalData::RegisterNode(const std::string& node_name) {
    //拿到node_name的哈希值
    auto id  = Hash(node_name);
    //检查hashmap中是否含有id
    while (node_id_map_.Has(id))
    {
       //如果id存在
       std::string* name = nullptr;
       node_id_map_.Get(id, &name);
       if(node_name == *name){
        break;
       }
       //说明有其他node_name和当前的哈希值相等，出现了哈希碰撞，将id++
       ++id;
       AWARN << " Node name hash collision: " << node_name << " <=> " << *name;
    }
    //确保node_name是一个唯一的id
    node_id_map_.Set(id , node_name);
    return id;
}

/* 注册Task，返回id */
uint64_t GlobalData::RegisterTaskName(const std::string& task_name) {
    auto id = Hash(task_name);
    while(task_id_map_.Has(id)) {
        std::string* name = nullptr;
        task_id_map_.Get(id, &name);
        if(task_name == *name) break;
        ++id;
        AWARN << "Task name hash collision: " << task_name << "<=>" << *name;
    }
    task_id_map_.Set(id, task_name);
    return id;
}

/* 返回Task名称，通过ID */
std::string GlobalData::GetTaskNameById(uint64_t id) {
    std::string* task_name = nullptr;
    if(task_id_map_.Get(id, &task_name)) return *task_name;
    return KEmptyString;
}

/* 初始化host name、ip */
void GlobalData::InitHostInfo() {
    char host_name[1024];
    gethostname(host_name, sizeof(host_name));
    host_name_ = host_name;

    host_ip_ = "127.0.0.1";

    const char* ip_env = getenv("RCMW_IP");
    if(ip_env != nullptr) {
        std::string ip_env_str(ip_env);
        std::string starts = ip_env_str.substr(0, 3);
        if(starts != "127") {
            host_ip_ = ip_env_str;
            return;
        }
    }

    ifaddrs* ifaddr = nullptr;
    if(getifaddrs(&ifaddr) != 0) 
        AERROR << "getifaddrs failed, we will use 127.0.0.1 as host ip.";
    for(ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
        if(ifa->ifa_addr == nullptr) continue;
        int family = ifa->ifa_addr->sa_family;
        if(family != AF_INET) continue;

        char addr[NI_MAXHOST] = { 0 };
        if(getnameinfo(ifa->ifa_addr, sizeof(sockaddr_in), addr, 
                NI_MAXHOST, NULL, 0, NI_NUMERICHOST) != 0) {
            continue;
        }
        std::string tmp_ip(addr);
        std::string starts = tmp_ip.substr(0, 3);
        if(starts != "127") {
            host_ip_ = tmp_ip;
            break;
        }
    }
    freeifaddrs(ifaddr);
}

} // common
} // rcmw
} // hnu
