/**
 * @brief 线程池
 * @date  2025.11.04
 */

#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <thread>
#include <functional>
#include <atomic>
#include <vector>
#include <future>
#include <iostream>
#include "bounded_queue.h"

namespace hnu  {
namespace rcmw {
namespace base {

class ThreadPool {
public:
    explicit ThreadPool(std::size_t thread_num, std::size_t max_task_num = 1000);
    template <typename F, typename... Args>
    auto Enqueue(F&& f, Args&&... args)
        ->std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();

private:
    std::vector<std::thread> workers_;                  // 线程vector
    BoundedQueue<std::function<void()>> task_queue_;    // 任务队列
    std::atomic_bool stop_;                             // 停止？
};

/* 构造函数：初始化任务队列、线程vector */
inline ThreadPool::ThreadPool(std::size_t threads, std::size_t max_task_num) : stop_(false) {
    if(!task_queue_.Init(max_task_num, new BlockWaitStrategy())) {
        throw std::runtime_error("Task queue init failed.");
    }
    workers_.reserve(threads);
    for(size_t i=0; i<threads; ++i) {
        workers_.emplace_back([this]{
            while (!stop_) {
                std::function<void()> task;
                if(task_queue_.WaitDequeue(&task)) {
                    task();
                }
            }
        });
    }
}

/* 任务添加 */
template <typename F, typename... Args>
auto ThreadPool::Enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
    // 推导返回类型
    using return_type = typename std::result_of<F(Args...)>::type;
    // 任务封装
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    // 返回future对象
    std::future<return_type> res = task->get_future();
    if(stop_) {
        return std::future<return_type>();
    }
    // 将任务添加到任务队列
    task_queue_.Enqueue([task]() { (*task)(); });
    return res;
}

inline ThreadPool::~ThreadPool() {
    if(stop_.exchange(true)) {
        return;
    }
    task_queue_.BreakAllWait();
    for(std::thread& worker : workers_) {
        worker.join();
    }
}

} // base
} // rcmw
} // hnu

#endif
