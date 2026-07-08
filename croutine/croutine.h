/**
 * @brief 协程类
 * @date 2025.12.20
 */

#ifndef _RCMW_ROUTINE_H_
#define _RCMW_ROUTINE_H_

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include "rcmw/logger/log.h"
#include "rcmw/croutine/croutine_context.h"

namespace hnu       {
namespace rcmw      {
namespace croutine  {

using RoutineFuc = std::function<void()>;
using Duration = std::chrono::microseconds;

enum class RoutineState { READY, FINISHED, SLEEP, IO_WAIT, DATA_WAIT };

class CRoutine {
public:
    explicit CRoutine(const RoutineFuc& func);
    virtual ~CRoutine();

    RoutineState Resume();
    static void Yield();
    static void Yield(const RoutineState& state);

    static void SetMainContext(const std::shared_ptr<RoutineContext>& context);
    static char **GetMainStack();

    static CRoutine* GetCurrentRoutine();

    RoutineContext* GetContext();
    char** GetStack();

    bool Acquire();
    void Release();

    void SetUpdateFlag();
    RoutineState UpdateState();

    void Run();

    void Stop();
    
    void Wake();
    void HangUp();
    void Sleep(const Duration& sleep_duration);
    std::chrono::steady_clock::time_point wake_time() const;

    RoutineState state() const { return state_; }
    void set_state(const RoutineState& state) { state_ = state; }

    uint64_t id() const { return id_; }
    void set_id(uint64_t id) { id_ = id; }

    const std::string &name() const { return name_; }
    void set_name(const std::string& name) { name_ = name; }

    int processor_id() const { return process_id_; }
    void set_processor_id(int processor_id) { process_id_ = processor_id; }

    uint32_t priority() const { return priority_; }
    void set_priority(uint32_t priority) { priority_ = priority; }

    void set_group_name(const std::string& group_name) { group_name_  = group_name; }
    const std::string& group_name() { return group_name_; }

private:
    CRoutine(CRoutine &) = delete;
    CRoutine &operator=(CRoutine& other) = delete;

    std::string name_;
    std::chrono::steady_clock::time_point wake_time_ = 
        std::chrono::steady_clock::now();
    
    RoutineFuc func_;
    RoutineState state_;
    std::shared_ptr<RoutineContext> context_;

    std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
    std::atomic_flag updated_ = ATOMIC_FLAG_INIT;

    bool force_stop_ = false;

    int process_id_ = -1;
    uint32_t priority_ = 0;
    uint64_t id_ = 0;

    std::string group_name_;

    static thread_local CRoutine * current_routine_;
    static thread_local char* main_stack_;
};

/**
 * @brief 指定协程状态并回到主线程
 * @param state 指定协程状态
 */
inline void CRoutine::Yield(const RoutineState& state) {
    auto routine = GetCurrentRoutine();
    routine->set_state(state);
    SwapContext(GetCurrentRoutine()->GetStack(), GetMainStack());
}

/**
 * @brief 回到主线程
 */
inline void CRoutine::Yield() {
    SwapContext(GetCurrentRoutine()->GetStack(), GetMainStack());
}

/**
 * @brief 获取当前协程
 */
inline CRoutine* CRoutine::GetCurrentRoutine() { return current_routine_; }

/**
 * @brief 获取主栈指针
 */
inline char** CRoutine::GetMainStack() { return &main_stack_; }

/**
 * @brief 获取协程上下文
 */
inline RoutineContext* CRoutine::GetContext() { return context_.get(); }

/**
 * @brief 获取上下文栈指针
 */
inline char** CRoutine::GetStack() { return &(context_->sp); }

/**
 * @brief 运行协程执行体
 */
inline void CRoutine::Run() { func_(); }

/**
 * @brief 获取协程wake_time_
 */
inline std::chrono::steady_clock::time_point CRoutine::wake_time() const { return wake_time_; }

/**
 * @brief 设置协程状态为READY
 */
inline void CRoutine::Wake() { state_ = RoutineState::READY; } 

/**
 * @brief 设置协程状态为DATA_WAIT
 */
inline void CRoutine::HangUp() { state_ = RoutineState::DATA_WAIT; }

/**
 * @brief 设置协程状态为睡眠
 * @param sleep_duration 睡眠时间
 */
inline void CRoutine::Sleep(const Duration& sleep_duration) {
    wake_time_ = std::chrono::steady_clock::now() + sleep_duration;
    CRoutine::Yield(RoutineState::SLEEP);
}

/**
 * @brief 更新协程状态
 * @return 返回更新后的状态
 */
inline RoutineState CRoutine::UpdateState() {
    if(state_ == RoutineState::SLEEP && std::chrono::steady_clock::now() > wake_time_) {
        state_ = RoutineState::READY;
        return state_;
    }
    if(!updated_.test_and_set(std::memory_order_release)) {
        if(state_ == RoutineState::DATA_WAIT || state_ == RoutineState::IO_WAIT) {
            state_ = RoutineState::READY;
        }
    }
    return state_;
}

/**
 * @brief 协程锁加锁
 * @return true 加锁成功，false 加锁失败
 */
inline bool CRoutine::Acquire() { 
    return !lock_.test_and_set(std::memory_order_acquire);
}

/**
 * @brief 协程锁解锁
 */
inline void CRoutine::Release() {
    return lock_.clear(std::memory_order_release);
}

/**
 * @brief 设置协程状态更新标志位为：false
 */
inline void CRoutine::SetUpdateFlag() {
    updated_.clear(std::memory_order_release);
}

} // croutine
} // rcmw
} // hnu

#endif
