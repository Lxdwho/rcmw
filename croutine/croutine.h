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
    /**
     * @brief 构造函数：使用call_once实现了单例，并从对象池中取出一个协程上下文
     * @param func 协程执行函数
     */
    explicit CRoutine(const RoutineFuc& func);
    virtual ~CRoutine();

    /**
     * @brief 回到主线程
     */
    static void Yield();
    
    /**
     * @brief 指定协程状态并回到主线程
     * @param state 指定协程状态
     */
    static void Yield(const RoutineState& state);

    /**
     * @brief 获取获取主栈
     */

    static void SetMainContext(const std::shared_ptr<RoutineContext>& context);
    /**
     * @brief 获取当前协程
     */
    static CRoutine* GetCurrentRoutine();

    static char **GetMainStack();

    /**
     * @brief 添加锁？
     * @return true 加锁成功，false 加锁失败
     */
    bool Acquire();

    /**
     * @brief 释放锁？
     */
    void Release();

    /**
     * @brief 设置协程状态更新标志位为：false
     */
    void SetUpdateFlag();

    RoutineState Resume();

    /**
     * @brief 更新协程状态
     * @return 返回更新后的状态
     */
    RoutineState UpdateState();

    /**
     * @brief 获取协程上下文
     */
    RoutineContext* GetContext();

    /**
     * @brief 获取上下文栈指针
     */
    char** GetStack();

    /**
     * @brief 运行协程执行体
     */
    void Run();

    /**
     * @brief 停止协程，将状态为置为false
     */
    void Stop();

    /**
     * @brief 设置协程状态为READY
     */
    void Wake();

    /**
     * @brief 设置协程状态为DATA_WAIT
     */
    void HangUp();

    /**
     * @brief 设置协程状态为睡眠
     * @param sleep_duration 睡眠时间
     */
    void Sleep(const Duration& sleep_duration);

    /**
     * @brief 获取协程状态
     */
    RoutineState state() const;

    /**
     * @brief 设置协程状态
     */
    void set_state(const RoutineState& state);

    uint64_t id() const;
    void set_id(uint64_t id);

    const std::string &name() const;
    void set_name(const std::string& name);

    int processor_id() const;
    void set_processor_id(int processor_id);

    uint32_t priority() const;
    void set_priority(uint32_t priority);

    /**
     * @brief 获取协程wake_time_
     */
    std::chrono::steady_clock::time_point wake_time() const;

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

inline void CRoutine::Yield(const RoutineState& state) {
    auto routine = GetCurrentRoutine();
    routine->set_state(state);
    SwapContext(GetCurrentRoutine()->GetStack(), GetMainStack());
}

inline void CRoutine::Yield() {
    SwapContext(GetCurrentRoutine()->GetStack(), GetMainStack());
}

inline CRoutine* CRoutine::GetCurrentRoutine() { return current_routine_; }

inline char** CRoutine::GetMainStack() { return &main_stack_; }

inline RoutineContext* CRoutine::GetContext() { return context_.get(); }

inline char** CRoutine::GetStack() { return &(context_->sp); }

inline void CRoutine::Run() { func_(); }

inline void CRoutine::set_state(const RoutineState &state) { state_ = state; }

inline RoutineState CRoutine::state() const { return state_; }

inline std::chrono::steady_clock::time_point CRoutine::wake_time() const { return wake_time_; }

inline void CRoutine::Wake() { state_ = RoutineState::READY; } 

inline void CRoutine::HangUp() { state_ = RoutineState::DATA_WAIT; }

inline void CRoutine::Sleep(const Duration& sleep_duration) {
    wake_time_ = std::chrono::steady_clock::now() + sleep_duration;
    CRoutine::Yield(RoutineState::SLEEP);
}

inline uint64_t CRoutine::id() const { return id_; }
inline void CRoutine::set_id(uint64_t id) { id_ = id; }

inline const std::string& CRoutine::name() const { return name_; }
inline void CRoutine::set_name(const std::string& name) { name_ = name; }

inline int CRoutine::processor_id() const { return process_id_; }
inline void CRoutine::set_processor_id(int processor_id) { process_id_ = processor_id; }

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

inline uint32_t CRoutine::priority() const { return priority_; }
inline void CRoutine::set_priority(uint32_t priority) { priority_ = priority; }

inline bool CRoutine::Acquire() { 
    return !lock_.test_and_set(std::memory_order_acquire);
}

inline void CRoutine::Release() {
    return lock_.clear(std::memory_order_release);
}

inline void CRoutine::SetUpdateFlag() {
    updated_.clear(std::memory_order_release);
}

} // croutine
} // rcmw
} // hnu

#endif
