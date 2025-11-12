/**
 * @brief 共享内存块锁机制？
 * @date 2025.11.12
 */

#include "block.h"
#include "rcmw/logger/log.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

const int32_t Block::KRWLockFree = 0;
const int32_t Block::KWriteExclusive = -1;
const int32_t Block::KMaxTryLockTimes = 5;

Block::Block() : msg_size_(0), msg_info_size_(0) {}

Block::~Block() {}


bool Block::TryLockForWrite() {
    int32_t rw_lock_free = KRWLockFree;
    if(!lock_num_.compare_exchange_weak(rw_lock_free, KWriteExclusive,
            std::memory_order_acq_rel, std::memory_order_relaxed)) {
        ADEBUG << "lock_num_: " << lock_num_.load();
        return false;
    }
    return true;
}

bool Block::TryLockForRead() {
    int32_t lock_num = lock_num_.load();
    if(lock_num < KRWLockFree) {
        AINFO << "block is being written";
        return false;
    }
    int32_t try_times = 0;
    while(!lock_num_.compare_exchange_weak(lock_num, lock_num+1, 
            std::memory_order_acq_rel, std::memory_order_relaxed)) {
        ++try_times;
        if(try_times == KMaxTryLockTimes) {
            AINFO << "fail to add read lock num, curr num: " << lock_num;
            return false;
        }
        lock_num = lock_num_.load();
        if(lock_num < KRWLockFree) {
            AINFO << "block is begin written";
            return false;
        }
    }
    return true;
}

void Block::ReleaseWriteLock() { lock_num_.fetch_add(1); }

void Block::ReleaseReadLock() { lock_num_.fetch_sub(1); }

} // transport
} // rcmw
} // hnu
