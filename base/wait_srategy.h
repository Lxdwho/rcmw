/**
 * @brief  线程等待策略
 * @date   2025.11.03
 * @bug    EmptyWait中传入了break_all_wait，
 *         用于防止线程由于唤醒时机问题进入等待无法退出
 */

#ifndef _WAIT_STRATEGY_H_
#define _WAIT_STRATEGY_H_

#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>


namespace hnu  {
namespace rcmw {
namespace base {

class WaitStrategy {
public:
    virtual void NotifyOne() {}
    virtual void BreakAllWait() {}
    virtual bool EmptyWait(volatile bool& break_all_wait) = 0;
    virtual ~WaitStrategy() {}
};

/* 阻塞等待策略 */
class BlockWaitStrategy : public WaitStrategy {
public:
    BlockWaitStrategy() {}
    void NotifyOne() override { cv_.notify_one(); }
    bool EmptyWait(volatile bool& break_all_wait) override {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&]{ return break_all_wait; });
        return true;
    }
    void BreakAllWait() override { cv_.notify_all(); }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
};

/* 睡眠等待策略 */
class SleepWaitStrategy : public WaitStrategy {
public:
    SleepWaitStrategy() {};
    explicit SleepWaitStrategy(uint64_t sleep_time_us) : sleep_time_us_(sleep_time_us) {}
    bool EmptyWait(volatile bool& break_all_wait) override {
        std::this_thread::sleep_for(std::chrono::microseconds(sleep_time_us_));
        return true;
    }
    void SetSleepTimeMicroSeconds(uint64_t sleep_time_us) {
        sleep_time_us_ = sleep_time_us;
    }

private:
    uint64_t sleep_time_us_ = 10000;
};

/* 调度等待策略 */
class YieldWaitStrategy : public WaitStrategy {
public:
    YieldWaitStrategy() {}
    bool EmptyWait(volatile bool& break_all_wait) override {
        std::this_thread::yield();
        return true;
    }
};

/* ??等待策略 */
class BusySpinWaitStrategy : public WaitStrategy {
public:
    BusySpinWaitStrategy() {};
    bool EmptyWait(volatile bool& break_all_wait) override { return true; }
};

/* 超时等待策略 */
class TimeoutBlockWaitStrategy : public WaitStrategy {
public:
    TimeoutBlockWaitStrategy() {}
    explicit TimeoutBlockWaitStrategy(uint64_t timeout) : 
        time_out_(std::chrono::milliseconds(timeout)) {}
    void NotifyOne() override { cv_.notify_one(); }

    bool EmptyWait(volatile bool& break_all_wait) override {
        std::unique_lock<std::mutex> lock(mutex_);
        if(cv_.wait_for(lock, time_out_) == std::cv_status::timeout) 
            return false;
        return true;
    }

    void BreakAllWait() override { cv_.notify_all(); };

    void SetTimeout(uint64_t timeout) {
        time_out_ = std::chrono::milliseconds(timeout);
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::chrono::milliseconds time_out_;
};

} // base
} // rcme
} // hnu

#endif
