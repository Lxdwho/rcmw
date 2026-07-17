/**
 * @brief 协程类
 * @date 2025.12.20
 */

#include "croutine/croutine.h"
#include "common/global_data.h"
#include "logger/log.h"
#include "base/concurrent_object_pool.h"
#include <algorithm>

namespace hnu       {
namespace rcmw      {
namespace croutine  {

thread_local CRoutine* CRoutine::current_routine_ = nullptr;
thread_local char* CRoutine::main_stack_ = nullptr;

namespace {
    std::shared_ptr<base::CCObjectPool<RoutineContext>> context_pool = nullptr;
    std::once_flag pool_init_flag;
    /**
     * @brief 协程入口函数，应当在Run函数中死循环，否则跑飞
     * @param arg 传入协程本身
     */
    void CRoutineEntry(void* arg) {
        CRoutine *r = static_cast<CRoutine*>(arg);
        r->Run();
        CRoutine::Yield(RoutineState::FINISHED);
    }
}

/**
 * @brief 构造函数：使用call_once实现了单例对象池，并从对象池中取出一个协程上下文
 * @param func 协程执行函数
 */
CRoutine::CRoutine(const RoutineFuc &func) : func_(func) {
    /* 实例化协程上下文对象池 */
    std::call_once(pool_init_flag, [&]{
        uint32_t routine_num = common::GlobalData::Instance()->ComponentNums();
        auto &global_conf = common::GlobalData::Instance()->Config();
        if(global_conf.scheduler_conf.routine_num) {
            routine_num = std::max(routine_num, global_conf.scheduler_conf.routine_num);
        }
        context_pool.reset(new base::CCObjectPool<RoutineContext>(routine_num));
    });

    /* 取一个上下文，为nullptr则实例化 */
    context_ = context_pool->GetObject();
    if(context_ == nullptr) {
        AWARN << "Maximum routine context number exceeded! Please check \
                    [routine_num] in config file.";
        context_.reset(new RoutineContext());
    }

    /* 初始化上下文 */
    MakeContext(CRoutineEntry, this, context_.get());
    state_ = RoutineState::READY;
    updated_.test_and_set(std::memory_order_release);
}

CRoutine::~CRoutine() { context_ = nullptr; }

/**
 * @brief 运行协程，检查状态后换栈
 * @return 协程执行完/不满足要求退出后，返回协程状态
 */
RoutineState CRoutine::Resume() {
    if(cyber_unlikely(force_stop_)) {
        state_ = RoutineState::FINISHED;
        return state_;
    }

    if(cyber_unlikely(state_ != RoutineState::READY)) {
        AERROR << "Invalid Routine State!";
        return state_;
    }
    /* 执行协程，换栈，回归 */
    current_routine_ = this;
    SwapContext(GetMainStack(), GetStack());
    current_routine_ = nullptr;
    return state_;
}

/**
 * @brief 停止协程，将状态为置为false
 */
void CRoutine::Stop() { force_stop_ = true; }

} // croutine
} // rcmw
} // hnu
