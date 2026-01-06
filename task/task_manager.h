/**
 * @brief 
 * @date 2026.01.06
 */

#ifndef _RCMW_TASK_TASK_MANAGER_H_
#define _RCMW_TASK_TASK_MANAGER_H_

#include <future>
#include <atomic>
#include <memory>
#include <vector>
#include <type_traits>
#include <functional>
#include "rcmw/base/bounded_queue.h"
#include "rcmw/common/macros.h"
#include "rcmw/scheduler/scheduler_factory.h"

namespace hnu   {
namespace rcmw  {

class TaskManager {
public:
    virtual ~TaskManager();
    void Shutdown();
    template <typename F, typename... Args>
    auto Enqueue(F&& func, Args&&... args)->std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;
        auto new_task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(func), std::forward<Args>(args)...));
        if(!stop_.load()) {
            task_queue_->Enqueue([new_task]() { (*new_task)(); });
            for(auto& task : tasks_) {
                scheduler::Instance()->NotifyTask(task);
            }
        }
        std::future<return_type> res(new_task->get_future());
        return res;
    }
private:
    uint32_t num_threads_ = 0;
    uint32_t task_queue_size_ = 1000;
    std::atomic<bool> stop_ = {false};
    std::vector<uint64_t> tasks_;
    std::shared_ptr<base::BoundedQueue<std::function<void()>>> task_queue_;
    DECLARE_SINGLETON(TaskManager)
};

} // rcmw
} // hnu

#endif
