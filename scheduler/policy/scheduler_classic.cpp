/**
 * @brief classic策略
 * @date 2025.12.30
 */

#include "rcmw/scheduler/policy/scheduler_classic.h"
#include "rcmw/scheduler/policy/classic_context.h"
#include "rcmw/scheduler/processor.h"
#include "rcmw/common/global_data.h"
#include "rcmw/common/environment.h"
#include "rcmw/config/conf_parse.h"
#include "rcmw/config/rcmw_conf.h"
#include "rcmw/common/file.h"

namespace hnu       {
namespace rcmw      {
namespace scheduler {

using hnu::rcmw::common::GetAbsolutePath;
using hnu::rcmw::common::GlobalData;
using hnu::rcmw::common::PathExist;
using hnu::rcmw::common::WorkRoot;
using hnu::rcmw::base::WriteLockGuard;
using hnu::rcmw::base::ReadLockGuard;
using hnu::rcmw::config::GetRcmwConfFromFile;
using hnu::rcmw::croutine::RoutineState;

/**
 * @brief 构造函数
 */
SchedulerClassic::SchedulerClassic() {
    /* 获取配置文件信息 */
    std::string conf("conf/");
    conf.append(GlobalData::Instance()->ProcessGroup()).append(".conf");
    auto cfg_file = GetAbsolutePath(WorkRoot(), conf);

    /* 读取配置文件 */
    hnu::rcmw::config::RcmwConfig cfg;
    if(PathExist(cfg_file) && GetRcmwConfFromFile(cfg_file, &cfg)) {
        /* 获取所有threads配置 */
        for(auto& thr : cfg.scheduler_conf.threads) 
            inner_thr_confs_[thr.name] = thr;
        
        /* 设置调度器线程亲和性 */
        if(!cfg.scheduler_conf.process_level_cpuset.empty()) {
            process_level_cpuset_ = cfg.scheduler_conf.process_level_cpuset;
            AINFO << "scheduler_conf.process_level_cpuset: " 
                  << cfg.scheduler_conf.process_level_cpuset;
            ProcessLevelResourceControl();
        }

        /* 获取classic_conf配置 */
        classic_conf_ = cfg.scheduler_conf.classic_conf;
        for(auto& group : classic_conf_.groups) {
            auto group_name = group.name;
            for(auto task : group.tasks) {
                task.group_name = group.name;
                cr_confs_[task.name] = task;
                AINFO << "cr_confs_[" << task.name << "]";
            }
        }
    }
    else {  /* 配置文件无效，则执行下面的配置 */
        uint32_t proc_num = 2;
        auto& global_conf = GlobalData::Instance()->Config();
        if(global_conf.scheduler_conf.default_proc_num)
            proc_num = global_conf.scheduler_conf.default_proc_num;
        task_pool_size_ = proc_num;
        classic_conf_.groups.emplace_back();
        auto& sched_group = classic_conf_.groups.back();
        sched_group.name = DEFAULT_GROUP_NAME;
        sched_group.processor_num = proc_num;
    }
    CreateProcessor();
}

/**
 * @brief 移除一个协程
 */
bool SchedulerClassic::RemoveCRoutine(uint64_t crid) {
    /* 获取对应协程的锁，为nullptr则new一个 */
    MutexWrapper* wrapper = nullptr;
    if(!id_map_mutex_.Get(crid, &wrapper)) {
        std::lock_guard<std::mutex> wl_lg(cr_wl_mtx_);
        if(!id_map_mutex_.Get(crid, &wrapper)) {
            wrapper = new MutexWrapper();
            id_map_mutex_.Set(crid, wrapper);
        }
    }

    /* 加锁移除协程？？？？ */
    std::lock_guard<std::mutex> lg(wrapper->Mutex());
    std::shared_ptr<CRoutine> cr = nullptr;
    {
        WriteLockGuard<AtomicRWLock> lk(id_cr_lock_);
        if(id_cr_.find(crid) != id_cr_.end()) {
            cr = id_cr_[crid];
            id_cr_[crid]->Stop();
            id_cr_.erase(crid);
        }
        else return false;
    }
    return ClassicContext::RemoveCRoutine(cr);
}

/**
 * @brief 移除Task对应的协程
 */
bool SchedulerClassic::RemoveTask(const std::string& name) {
    if(cyber_unlikely(stop_)) return true;
    auto crid = GlobalData::GenerateHashId(name);
    return RemoveCRoutine(crid);
}

/**
 * @brief 分发协程到抽象CPU上下文中
 * @param cr 分发对象
 */
bool SchedulerClassic::DispatchTask(const std::shared_ptr<CRoutine>& cr) {
    /* 获取对应协程的锁，为nullptr则new一个 */
    MutexWrapper* wrapper = nullptr;
    if(!id_map_mutex_.Get(cr->id(), &wrapper)) {
        std::lock_guard<std::mutex> wl_lg(cr_wl_mtx_);
        if(!id_map_mutex_.Get(cr->id(), &wrapper)) {
            wrapper = new MutexWrapper();
            id_map_mutex_.Set(cr->id(), wrapper);
        }
    }

    /* 将协程加入id_cr_ */
    std::lock_guard<std::mutex> lg(wrapper->Mutex());
    {
        WriteLockGuard<AtomicRWLock> lk(id_cr_lock_);
        if(id_cr_.find(cr->id()) != id_cr_.end()) return false;
        id_cr_[cr->id()] = cr;
    }

    /* 配置协程属性 */
    if(cr_confs_.find(cr->name()) != cr_confs_.end()) {
        ClassicTask task = cr_confs_[cr->name()];
        cr->set_priority(task.prio);
        cr->set_group_name(task.group_name);
    }
    else cr->set_group_name(classic_conf_.groups[0].name);

    if(cr->priority() >= MAX_PRIO) {
        AWARN << cr->name() << " prio is greater than MAX_PRIO[" << MAX_PRIO << "]";
        cr->set_priority(MAX_PRIO - 1);
    }

    /* 将协程加入到抽象CPU上下文中的对应协程队列中 */
    {
        WriteLockGuard<AtomicRWLock> lk(ClassicContext::
            rq_locks_[cr->group_name()].at(cr->priority()));
        ClassicContext::cr_group_[cr->group_name()]
            .at(cr->priority()).emplace_back(cr);
        ADEBUG << "Add " << cr->group_name() << " to" 
               << "ClassicContext::cr_group_[" << cr->group_name() 
               << "].at(" << cr->priority() << ").emplace_back("
               << cr << ")";
    }

    /* 唤醒对应协程组别中的一个协程 */
    ClassicContext::Notify(cr->group_name());
    return true;
}

/**
 * @brief 创建抽象CPU
 */
void SchedulerClassic::CreateProcessor() {
    /* 依次创建每个组别 */
    for(auto& group : classic_conf_.groups) {
        auto& group_name = group.name;
        
        auto proc_num = group.processor_num;
        if(task_pool_size_ == 0) task_pool_size_ = proc_num;

        auto& affinity = group.affinity;
        auto& processor_policy = group.processor_policy;
        auto processor_prio = group.processor_prio;

        /* 解析cpu亲和性 */
        std::vector<int> cpuset;
        ParseCpuset(group.cpuset, &cpuset);

        /* 为每个抽象CPU创建对应的线程：所有的线程为组内的task服务 */
        for(uint32_t i=0; i<proc_num; i++) {
            auto ctx = std::make_shared<ClassicContext>(group_name);
            pctxs_.emplace_back(ctx);

            auto proc = std::make_shared<Processor>();
            proc->BindContext(ctx);

            /* 设置线程亲和性、调度策略 */
            SetSchedAffinity(proc->Thread(), cpuset, affinity, i);
            SetSchedPolicy(proc->Thread(), processor_policy, processor_prio, proc->Tid());
            processors_.emplace_back(proc);
        }
    }
}

/**
 * @brief 通知抽象CPU，最终通知到对应协程的组别，唤醒一个processor
 */
bool SchedulerClassic::NotifyProcessor(uint64_t crid) {
    if(cyber_unlikely(stop_)) return true;

    {
        ReadLockGuard<AtomicRWLock> lk(id_cr_lock_);
        if(id_cr_.find(crid) != id_cr_.end()) {
            auto cr = id_cr_[crid];
            if(cr->state() == RoutineState::DATA_WAIT || 
                    cr->state() == RoutineState::IO_WAIT) {
                cr->SetUpdateFlag();
            }
            ClassicContext::Notify(cr->group_name());
            return true;
        }
    }
    return false;
}

// friend Scheduler* Instance() {}

} // scheduler
} // rcmw
} // hnu
