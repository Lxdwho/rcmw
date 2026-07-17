/**
 * @brief 抽象CPU类
 * @date 2025.12.27
 */

#include "scheduler/processor.h"
#include "common/global_data.h"
#include "logger/log.h"
#include "croutine/croutine.h"
#include "time/time.h"
#include <chrono>
#include <sched.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace hnu       {
namespace rcmw      {
namespace scheduler {

using hnu::rcmw::common::GlobalData;

Processor::Processor() { running_.store(true); }

Processor::~Processor() { Stop(); }

/**
 * @brief 抽象CPU执行体，反复取就绪协程执行
 * @details 为防止死锁采取通知模式，取不到就绪协程后阻塞等待
 */
void Processor::Run() {
    /* 获取内核级线程id */
    tid_.store(static_cast<int>(syscall(SYS_gettid)));
    snapshot_->processor_id.store(tid_);

    /* 从抽象CPU上下文中取READY协程执行，上下文不存在或无可用协程则等待条件变量 */
    while(cyber_likely(running_.load())) {
        if(cyber_likely(context_ != nullptr)) {
            auto croutine = context_->NextRoutine();
            if(croutine) {
                snapshot_->execute_start_time.store(hnu::rcmw::Time::Now().ToNanosecond());
                snapshot_->routine_name = croutine->name();
                croutine->Resume();
                croutine->Release();
            }
            else {
                snapshot_->execute_start_time.store(0);
                context_->Wait();
            }
        }
        else {
            std::unique_lock<std::mutex> lock(ctx_mutex_);
            cv_ctx_.wait_for(lock, std::chrono::milliseconds(10));
        }
    }
}

void Processor::Stop() {
    // 停止抽象CPU运行
    if(!running_.exchange(false)) return;
    // 停止抽象CPU上下文获取协程句柄
    if(context_) context_->Shutdown();
    // 唤醒陷入等待的抽象CPU线程
    cv_ctx_.notify_one();
    // 等待线程执行完最后的事务
    if(thread_.joinable()) thread_.join();
}

/**
 * @brief 绑定抽象CPU上下文，使用once_flag创建唯一的后台执行线程RUN
 */
void Processor::BindContext(const std::shared_ptr<ProcessorContext>& context) {
    context_ = context;
    std::call_once(thread_flag_, [this]() { thread_ = std::thread(&Processor::Run, this); });
}

/**
 * @brief 获取线程ID
 */
std::atomic<pid_t>& Processor::Tid() {
    while(tid_.load() == -1) {
        cpu_relax();
    }
    return tid_;
}

} // scheduler
} // rcmw
} // hnu
