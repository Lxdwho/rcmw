/**
 * @brief classic策略
 * @date 2025.12.29
 */

#include "rcmw/scheduler/policy/classic_context.h"

namespace hnu       {
namespace rcmw      {
namespace scheduler {

using hnu::rcmw::base::AtomicRWLock;
using hnu::rcmw::base::ReadLockGuard;
using hnu::rcmw::base::WriteLockGuard;
using hnu::rcmw::croutine::CRoutine;
using hnu::rcmw::croutine::RoutineState;

alignas(CACHELINE_SIZE) CR_GROUP ClassicContext::cr_group_;
alignas(CACHELINE_SIZE) RQ_LOCK_GROUP ClassicContext::rq_locks_;
alignas(CACHELINE_SIZE) GRP_WQ_CV ClassicContext::cv_wq_;
alignas(CACHELINE_SIZE) GRP_WQ_MUTEX ClassicContext::mtx_wq_;
alignas(CACHELINE_SIZE) NOTIFY_GRP ClassicContext::notify_grp_;

ClassicContext::ClassicContext() { InitGroup(DEFAULT_GROUP_NAME); }
ClassicContext::ClassicContext(const std::string& group_name) { InitGroup(group_name); }
void ClassicContext::InitGroup(const std::string& group_name) {
    mutil_pri_rq_   = &cr_group_[group_name];     // 组内的多优先级协程队列
    lq_             = &rq_locks_[group_name];     // 组内多优先级原子读写锁
    mtx_wrapper_    = &mtx_wq_[group_name];       // 该组的互斥锁
    cw_             = &cv_wq_[group_name];        // 该组的条件变量

    notify_grp_[group_name] = 0;                  // 该组的notify设为0
    current_grp_ = group_name;                    // 当前组的name
}

/**
 * @brief 根据优先级取READY协程
 * @return nullptr 未找到符合条件的协程 cr 返回符合条件协程
 */
std::shared_ptr<CRoutine> ClassicContext::NextRoutine() {
    if(cyber_unlikely(stop_.load())) return nullptr;

    /* 从当前组中按照优先级取就绪态的协程 */
    for(int i=MAX_PRIO-1; i>=0; --i) {
        /* 对group中的优先级i队列进行上锁 */
        ReadLockGuard<AtomicRWLock> lock(lq_->at(i));
    // std::cout << "are you ok????????????????????? " << i << std::endl;
        for(auto& cr : mutil_pri_rq_->at(i)) {
            /* 尝试持有协程 */
            if(!cr->Acquire()) continue;
            /* 更新协程状态 */
            if(cr->UpdateState() == RoutineState::READY) {
                return cr;
            }
            /* 协程不为就绪态则释放协程 */
            cr->Release();
        }
    }
    return nullptr;
}

/**
 * @brief 等待notify，1ms唤醒一次，等待条件notify_grp_[current_grp_] > 0
 */
void ClassicContext::Wait() {
    std::unique_lock<std::mutex> lock(mtx_wrapper_->Mutex());
    cw_->Cv().wait_for(lock, std::chrono::microseconds(1000),[&]() {
        return notify_grp_[current_grp_] > 0;
    });
    if(notify_grp_[current_grp_] > 0) notify_grp_[current_grp_]--;
}

/**
 * @brief 关闭，唤醒所有Wait，设置notify_grp_[current_grp_] 为 55
 */
void ClassicContext::Shutdown() {
    stop_.store(true);
    mtx_wrapper_->Mutex().lock();
    notify_grp_[current_grp_] = std::numeric_limits<unsigned char>::max();
    mtx_wrapper_->Mutex().unlock();
    cw_->Cv().notify_all();
}

/**
 * @brief 设置notify_grp_[current_grp_]++，唤醒一个processor
 */
void ClassicContext::Notify(const std::string& group_name) {
    /* 对group进行上锁 */
    (&mtx_wq_[group_name])->Mutex().lock();
    notify_grp_[group_name]++;
    (&mtx_wq_[group_name])->Mutex().unlock();
    /* 唤醒一个processor */
    cv_wq_[group_name].Cv().notify_one();
}

/**
 * @brief 删除指定协程
 */
bool ClassicContext::RemoveCRoutine(const std::shared_ptr<CRoutine>& cr) {
    auto grp = cr->group_name();
    auto prio = cr->priority();
    auto crid = cr->id();
    /* 对group的priorty优先级进行加锁 */
    WriteLockGuard<AtomicRWLock> lock(ClassicContext::rq_locks_[grp].at(prio));

    /* 取当前优先级下的所有协程 */
    auto& croutines = ClassicContext::cr_group_[grp].at(prio);
    for(auto it = croutines.begin(); it != croutines.end(); ++it) {
        /* 找到指定协程 */
        if((*it)->id() == crid) {
            auto cr = *it;
            cr->Stop();
            /* 尝试持有协程 */
            while(!cr->Acquire()) {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                AINFO << "waitting for task " << cr->name() << " completion.";
            }
            AINFO << "task " << cr->name() << " completion.";
            croutines.erase(it);
            cr->Release();
            return true;
        }
    }
    return false;
}

} // scheduler
} // rcmw
} // hnu
