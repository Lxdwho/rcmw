/**
 * @brief 重看、重写
 * @date 2025.11.14
 */

#ifndef _GLOBAL_DATA_H_
#define _GLOBAL_DATA_H_

#include "rcmw/base/atomic_hash_map.h"
#include "rcmw/base/atomic_rw_lock.h"
#include "rcmw/common/macros.h"
#include "rcmw/common/util.h"
#include "rcmw/config/rcmw_conf.h"
#include <string>

namespace hnu    {
namespace rcmw   {
namespace common {

using ::hnu::rcmw::base::AtomicHashMap;
using ::hnu::rcmw::config::RcmwConfig;

class GlobalData {
public:
    ~GlobalData();
    
    int ProcessId() const;

    void SetProcessGroup(const std::string& process_group);
    const std::string& ProcessGroup() const;

    void SetComponentNums(const int component_nums);
    int ComponentNums() const;

    const std::string & HostIp() const;
    const std::string & HostName() const;

    const RcmwConfig& Config() const;

    static uint64_t GenerateHashId(const std::string& name) { return common::Hash(name); }

    static std::string GetChannelById(uint64_t id);

    static uint64_t RegisterChannel(const std::string& channel);

    static uint64_t RegisterTaskName(const std::string& task_name);
    static std::string GetTaskNameById(uint64_t id);

private:
    void InitHostInfo();
    bool InitConfig();

    // global config
    RcmwConfig config_;

    // 运行机器的配置信息
    std::string host_ip_;
    std::string host_name_;

    //当前进程的信息
    int porcess_id_;
    std::string process_group_;

    int component_nums_ = 0;

    // run mode 暂时没有运行模式
     
    // sched policy info  暂时不支持调度
    std::string sched_name_ = "HNU_CMW_DEFAULT";  

    //在创建新的channel时会注册进此全局map
    static AtomicHashMap<uint64_t, std::string, 256> channel_id_map_;   //全局 channel_id_map_ 表
    static AtomicHashMap<uint64_t, std::string, 512> node_id_map_;
    
    static AtomicHashMap<uint64_t, std::string, 256> task_id_map_;

    //GlobalData为全局单例
    DECLARE_SINGLETON(GlobalData)

};

} // common
} // rcmw
} // hnu

#endif
