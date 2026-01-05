/**
 * @brief 调度器单例工厂
 * @date 2026.01.04
 */

#ifndef _RCMW_SCHEDULER_SCHEDULER_FACTORY_H_
#define _RCMW_SCHEDULER_SCHEDULER_FACTORY_H_

#include "rcmw/common/environment.h"
#include "rcmw/common/file.h"
#include "rcmw/common/global_data.h"
#include "rcmw/common/util.h"
#include "rcmw/scheduler/policy/scheduler_classic.h"
#include "rcmw/scheduler/scheduler.h"

namespace hnu       {
namespace rcmw      {
namespace scheduler {

Scheduler* Instance();
void CleanUp();

} // scheduler
} // rcmw
} // hnu

#endif
