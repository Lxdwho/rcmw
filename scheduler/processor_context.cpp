/**
 * @brief 
 * @date 2025.12.27
 */

#include "rcmw/scheduler/processor_context.h"

namespace hnu       {
namespace rcmw      {
namespace scheduler {

void ProcessorContext::Shutdown() {
    stop_.store(true);
}

} // scheduler
} // rcmw
} // hnu
