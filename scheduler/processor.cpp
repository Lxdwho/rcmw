/**
 * @brief 
 * @date 2025.12.27
 */

#include "rcmw/scheduler/processor.h"
#include "rcmw/common/global_data.h"
#include "rcmw/logger/log.h"
#include "rcmw/croutine/croutine.h"
#include "rcmw/time/time.h"
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

void Processor::Run() {
    /* 获取内核级线程id */
    tid_.store(static_cast<int>(syscall(SYS_gettid)));
    snapshot_->processor_id.store(tid_);

    /*  */
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
    if(!running_.exchange(false)) return;

    if(context_) context_->Shutdown();

    cv_ctx_.notify_one();

    if(thread_.joinable()) thread_.join();
}

void Processor::BindContext(const std::shared_ptr<ProcessorContext>& context) {
    context_ = context;
    std::call_once(thread_flag_, [this]() { thread_ = std::thread(&Processor::Run, this); });
}

std::atomic<pid_t>& Processor::Tid() {
    while(tid_.load() == -1) {
        cpu_relax();
    }
    return tid_;
}

} // scheduler
} // rcmw
} // hnu
