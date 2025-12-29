/**
 * @brief 调度基类，待看
 * @date 2025.12.27
 */

#include "rcmw/scheduler/scheduler.h"
#include "rcmw/common/environment.h"
#include "rcmw/common/file.h"
#include "rcmw/common/global_data.h"
#include "rcmw/common/util.h"
#include "rcmw/data/data_visitor/data_visitor.h"
#include "rcmw/scheduler/processor_context.h"
#include "rcmw/scheduler/processor.h"
#include <sched.h>
#include <utility>

namespace hnu       {
namespace rcmw      {
namespace scheduler {

using hnu::rcmw::common::GlobalData;

bool Scheduler::CreateTask(const RoutineFactory& factory, const std::string& name) {
    return CreateTask(factory.create_routine(), name, factory.GetDataVisitor());
}

bool Scheduler::CreateTask(std::function<void()>&& func, const std::string& name, 
                std::shared_ptr<DataVisitorBase> visitor = nullptr) {
    if(cyber_unlikely(stop_.load())) {
        ADEBUG << "scheduler is stoped, cannot create task!";
        return false;
    }

    auto task_id = GlobalData::RegisterTaskName(name);

    /* 创建协程 */
    auto cr = std::make_shared<CRoutine>(func);
    cr->set_id(task_id);
    cr->set_name(name);
    AINFO << "create croutine: " << name;

    /* 这是啥？ */
    if(!DispatchTask(cr)) return false;

    /* 这是啥？ */
    if(visitor != nullptr) {
        visitor->RegisterNotifyCallback([this, task_id]() {
            if(cyber_unlikely(stop_.load())) return;
            this->NotifyProcessor(task_id);
        });
    }
    return true;
}

bool Scheduler::NotifyTask(uint64_t crid) {
    if(cyber_unlikely(stop_.load())) return true;
    return NotifyProcessor(crid);
}

void Scheduler::Shutdown() {
    if(cyber_unlikely(stop_.load())) return;
    for(auto& ctx : pctxs_) ctx->Shutdown();
    
    std::vector<uint64_t> cr_list;
    {
        ReadLockGuard<AtomicRWLock> lock(id_cr_lock_);
        for(auto& cr : id_cr_) cr_list.emplace_back(cr.second->id());
    }

    for(auto& id : cr_list) RemoveCRoutine(id);

    for(auto& processor : processors_) processor->Stop();

    processors_.clear();
    pctxs_.clear();
}

void Scheduler::ProcessLevelResourceControl() {
    std::vector<int> cpus;
    ParseCpuset(process_level_cpuset_, &cpus);
    
    cpu_set_t set;
    CPU_ZERO(&set);
    
    for(const auto cpu : cpus) CPU_SET(cpu, &set);
    pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
}

void Scheduler::SetInnerThreadAttr(const std::string& name, std::thread* thr) {
    if(thr != nullptr && inner_thr_confs_.find(name) != inner_thr_confs_.end()) {
        /* 拿到配置 */
        auto th_conf = inner_thr_confs_[name];
        auto cpuset = th_conf.cpuset;

        /* 解析并设置CPU亲和性与策略 */
        std::vector<int> cpus;
        ParseCpuset(cpuset, &cpus);
        SetSchedAffinity(thr, cpus, "range");
        SetSchedPolicy(thr, th_conf.policy, th_conf.prio);
    }
}

void Scheduler::CheckSchedStatus() {
    std::string snap_info;
    auto now = Time::Now().ToNanosecond();
    for(auto processor : processors_) {
        auto snap = processor->ProcSnapshot();
        if(snap->execute_start_time.load()) {
            auto execute_time = (now - snap->execute_start_time.load()) / 1000000;
            snap_info.append(std::to_string(snap->processor_id.load()))
                     .append(":")
                     .append(snap->routine_name)
                     .append(":")
                     .append(std::to_string(execute_time));
        }
        else {
            snap_info.append(std::to_string(snap->processor_id.load())).append(":idle");
        }
        snap_info.append(", ");
    }
    snap_info.append("timestamp: ").append(std::to_string(now));
    AINFO << snap_info;
    snap_info.clear();
}

// static Scheduler* Instance();

} // scheduler
} // rcmw
} // hnu
