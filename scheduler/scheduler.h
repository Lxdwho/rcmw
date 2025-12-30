/**
 * @brief 调度基类
 * @date 2025.12.27
 */

#ifndef _RCMW_SCHEDULER_SCHEDULER_H_
#define _RCMW_SCHEDULER_SCHEDULER_H_

#include <unistd.h>
#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <thread>
#include <unordered_map>
#include <vector>
#include "rcmw/config/scheduler_conf.h"
#include "rcmw/logger/log.h"
#include "rcmw/common/macros.h"
#include "rcmw/common/types.h"
#include "rcmw/croutine/croutine.h"
#include "rcmw/croutine/croutine_factory.h"
#include "rcmw/scheduler/common/mutex_wrapper.h"
#include "rcmw/scheduler/common/pin_thread.h"

namespace hnu       {
namespace rcmw      {
namespace scheduler {

using hnu::rcmw::base::AtomicHashMap;
using hnu::rcmw::base::AtomicRWLock;
using hnu::rcmw::base::ReadLockGuard;
using hnu::rcmw::croutine::CRoutine;
using hnu::rcmw::croutine::RoutineFactory;
using hnu::rcmw::data::DataVisitorBase;
using hnu::rcmw::config::InnerThread;

class Processor;
class ProcessorContext;

class Scheduler {
public:
    virtual ~Scheduler() {};
    static Scheduler* Instance();

    /**
     * @brief 创建协程
     * @param factory 协程工厂
     * @param name 协程名称
     */
    bool CreateTask(const RoutineFactory& factory, const std::string& name);

    /**
     * @brief 创建协程
     * @param func 协程执行体
     * @param name 协程名称
     * @param visitor 协程对应的数据访问者？
     */
    bool CreateTask(std::function<void()>&& func, const std::string& name, 
                    std::shared_ptr<DataVisitorBase> visitor = nullptr);

    /**
     * @brief 通知任务
     * @param crid 协程id
     */
    bool NotifyTask(uint64_t crid);

    /**
     * @brief 停止调度，释放资源
     */
    void Shutdown();

    /**
     * @brief 资源等级控制
     */
    void ProcessLevelResourceControl();

    /**
     * @brief 线程属性设置
     * @param name 线程名称
     * @param thr 被设置线程
     */
    void SetInnerThreadAttr(const std::string& name, std::thread* thr);

    /**
     * @brief 检查调度状态并输出到日志
     */
    void CheckSchedStatus();

    /**
     * @brief 设置线程配置变量inner_thr_confs_
     */   
    void SetInnerThreadConfs(const std::unordered_map<std::string, InnerThread>& confs) {
        inner_thr_confs_ = confs;
    }

    /**
     * @brief 获取任务池大小
     */   
    uint32_t TaskPoolSize() { return task_pool_size_; }
    
    virtual bool RemoveTask(const std::string& name) = 0;
    virtual bool DispatchTask(const std::shared_ptr<CRoutine>& cr) = 0;
    virtual bool NotifyProcessor(uint64_t crid) = 0;
    virtual bool RemoveCRoutine(uint64_t crid) = 0;

protected:
    Scheduler() : stop_(false) {}

    AtomicRWLock id_cr_lock_;
    AtomicHashMap<uint64_t, MutexWrapper*> id_map_mutex_;
    std::mutex cr_wl_mtx_;

    std::unordered_map<uint64_t, std::shared_ptr<CRoutine>> id_cr_;
    std::vector<std::shared_ptr<ProcessorContext>> pctxs_;
    std::vector<std::shared_ptr<Processor>> processors_;

    std::unordered_map<std::string, InnerThread> inner_thr_confs_;

    std::string process_level_cpuset_;
    uint32_t proc_num_ = 0;
    uint32_t task_pool_size_ = 0;
    std::atomic<bool> stop_;
};

} // scheduler
} // rcmw
} // hnu

#endif
