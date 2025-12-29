/**
 * @brief 
 * @date 2025.12.27
 */

#ifndef _RCMW_SCHEDULER_PROCESSOR_CONTEXT_H_
#define _RCMW_SCHEDULER_PROCESSOR_CONTEXT_H_

#include <limits>
#include <memory>
#include <mutex>
#include "rcmw/base/macros.h"
#include "rcmw/croutine/croutine.h"

namespace hnu       {
namespace rcmw      {
namespace scheduler {

using croutine::CRoutine;

/**
 * @brief 执行单位上下文
 */
class ProcessorContext {
public:
    virtual void Shutdown();
    virtual std::shared_ptr<CRoutine> NextRoutine() = 0;
    virtual void Wait() = 0;
protected:
    std::atomic<bool> stop_{false};
};

} // scheduler
} // rcmw
} // hnu

#endif
