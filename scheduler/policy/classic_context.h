/**
 * @brief classic策略上下文
 * @date 2025.12.29
 */

#ifndef _RCMW_SCHEDULER_CLASSIC_CONTEXT_H_
#define _RCMW_SCHEDULER_CLASSIC_CONTEXT_H_

#include <array>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "rcmw/base/atomic_rw_lock.h"
#include "rcmw/croutine/croutine.h"
#include "rcmw/scheduler/common/cv_wrapper.h"
#include "rcmw/scheduler/common/mutex_wrapper.h"
#include "rcmw/scheduler/processor_context.h"

namespace hnu       {
namespace rcmw      {
namespace scheduler {

static constexpr uint32_t MAX_PRIO = 20;

#define DEFAULT_GROUP_NAME "default_grp"

using CROUTINE_QUEUE = std::vector<std::shared_ptr<CRoutine>>;

using MUTIL_PRIO_QUEUE = std::array<CROUTINE_QUEUE, MAX_PRIO>;
using CR_GROUP = std::unordered_map<std::string, MUTIL_PRIO_QUEUE>;

using LOCK_QUEUE = std::array<base::AtomicRWLock, MAX_PRIO>;
using RQ_LOCK_GROUP = std::unordered_map<std::string, LOCK_QUEUE>;

using GRP_WQ_MUTEX = std::unordered_map<std::string, MutexWrapper>;
using GRP_WQ_CV = std::unordered_map<std::string, CvWrapper>;
using NOTIFY_GRP = std::unordered_map<std::string, int>;

class ClassicContext : public ProcessorContext {
public:
    ClassicContext();
    explicit ClassicContext(const std::string& group_name);

    /**
     * @brief 按照优先级寻找到一个就绪态的协程，进行返回
     * @return nullptr 没找到就绪态协程
     */
    std::shared_ptr<CRoutine> NextRoutine() override;

    /**
     * @brief 等待信号量，对notify_grp_进行操作
     */
    void Wait() override;

    /**
     * @brief 唤醒所有线程，置stop_为true
     */
    void Shutdown() override;

    /**
     * @brief 唤醒一个线程
     */
    static void Notify(const std::string& group_name);
    
    /**
     * @brief 删除一个协程
     * @return true 删除成功，false 未找到该协程
     */
    static bool RemoveCRoutine(const std::shared_ptr<CRoutine>& cr);

    alignas(CACHELINE_SIZE) static CR_GROUP cr_group_;
    alignas(CACHELINE_SIZE) static RQ_LOCK_GROUP rq_locks_;
    alignas(CACHELINE_SIZE) static GRP_WQ_CV cv_wq_;
    alignas(CACHELINE_SIZE) static GRP_WQ_MUTEX mtx_wq_;
    alignas(CACHELINE_SIZE) static NOTIFY_GRP notify_grp_;

private:
    void InitGroup(const std::string& group_name);
    
    std::chrono::steady_clock::time_point wake_time_;
    bool need_sleep_ = false;

    MUTIL_PRIO_QUEUE* mutil_pri_rq_ = nullptr;
    LOCK_QUEUE* lq_ = nullptr;
    MutexWrapper* mtx_wrapper_ = nullptr;
    CvWrapper* cw_ = nullptr;

    std::string current_grp_;
};

} // scheduler
} // rcmw
} // hnu

#endif
