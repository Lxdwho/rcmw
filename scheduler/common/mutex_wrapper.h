/**
 * @brief 互斥锁封装
 * @date 2025.12.27
 */

#ifndef _RCMW_SCHEDULER_COMMON_MUTEX_WRAPPER_H_
#define _RCMW_SCHEDULER_COMMON_MUTEX_WRAPPER_H_

#include <mutex>

namespace hnu       {
namespace rcmw      {
namespace scheduler {

class MutexWrapper {
public:
    MutexWrapper& operator=(const MutexWrapper& other) = delete;
    std::mutex& Mutex() { return mutex_; }
private:
    mutable std::mutex mutex_;
};

} // scheduler
} // rcmw
} // hnu

#endif
