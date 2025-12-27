/**
 * @brief 条件变量封装
 * @date 2025.12.27
 */

#ifndef _RCMW_SCHEDULER_COMMON_CV_WRAPPER_H_
#define _RCMW_SCHEDULER_COMMON_CV_WRAPPER_H_

#include <condition_variable>

namespace hnu       {
namespace rcmw      {
namespace scheduler {

class CvWrapper {
public:
    CvWrapper& operator=(const CvWrapper& other) = delete;
    std::condition_variable& Cv() { return cv_; }
private:
    mutable std::condition_variable cv_;
};

} // scheduler
} // rcmw
} // hnu

#endif
