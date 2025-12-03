/**
 * @brief   原子读写锁仿写
 * @date    2025.10.30
 */

#ifndef ATOMIC_RW_LOCK_H_
#define ATOMIC_RW_LOCK_H_

#include "rw_lock_guard.h"
#include <atomic>
#include <thread>

namespace hnu  {
namespace rcmw {
namespace base {

class AtomicRWLock {
    friend class ReadLockGuard<AtomicRWLock>;
    friend class WriteLockGuard<AtomicRWLock>;
public:
    static const int32_t RW_LOCK_FREE = 0;
    static const int32_t WRITE_EXCLUSIVE = -1;
    static const uint32_t MAX_RETRY_TIMES = 5;
    AtomicRWLock() {}
    explicit AtomicRWLock(bool write_first) : write_first_(write_first) {}

private:
    void ReadLock();
    void WriteLock();

    void ReadUnlock();
    void WriteUnlock();

    AtomicRWLock(const AtomicRWLock& other) = delete;
    AtomicRWLock operator=(const AtomicRWLock& other) = delete;

    std::atomic<uint32_t> write_lock_wait_num_ = { 0 };
    std::atomic<int32_t> lock_num_ = { 0 };
    bool write_first_ = true;

};

inline void AtomicRWLock::ReadLock() {
    uint32_t retry_times = 0;
    int32_t lock_num = lock_num_.load();
    if(write_first_) {
        do {
            /* 锁被写线程持有， 或有写线程在等待 */
            while(lock_num < RW_LOCK_FREE || write_lock_wait_num_.load() > 0) {
                if(++retry_times == MAX_RETRY_TIMES) {
                    std::this_thread::yield();
                    retry_times = 0;
                }
                lock_num = lock_num_.load();
            }
        }
        while(!lock_num_.compare_exchange_weak(lock_num, lock_num + 1, 
                std::memory_order_acq_rel, std::memory_order_relaxed));
    }
    else {
        do {
            /* 锁被写线程持有，释放时跳出 */
            while (lock_num < RW_LOCK_FREE)
            {
                if(++retry_times == MAX_RETRY_TIMES) {
                    std::this_thread::yield();
                    retry_times = 0;
                }
                lock_num = lock_num_.load();
            }
        }
        while(!lock_num_.compare_exchange_weak(lock_num, lock_num + 1, 
                std::memory_order_acq_rel, std::memory_order_relaxed));
    }
}


inline void AtomicRWLock::WriteLock() {
    int32_t rw_lock_free = RW_LOCK_FREE;
    uint32_t retry_times = 0;
    write_lock_wait_num_.fetch_add(1);
    /* 锁空闲时加锁 */
    while(!lock_num_.compare_exchange_weak(rw_lock_free, WRITE_EXCLUSIVE, 
            std::memory_order_acq_rel, std::memory_order_relaxed)) {
        rw_lock_free = RW_LOCK_FREE;
        if(++retry_times == MAX_RETRY_TIMES) {
            std::this_thread::yield();
            retry_times = 0;
        }
    }
    write_lock_wait_num_.fetch_sub(1);
}


inline void AtomicRWLock::ReadUnlock() { lock_num_.fetch_sub(1); }

inline void AtomicRWLock::WriteUnlock() { lock_num_.fetch_add(1); }

} // namespace base
} // namespace rcmw
} // namespace hnu

#endif
