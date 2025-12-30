/**
 * @brief classic策略
 * @date 2025.12.30
 */

#ifndef _RCMW_SCHEDULER_SCHEDULER_CLASSIC_H_
#define _RCMW_SCHEDULER_SCHEDULER_CLASSIC_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "rcmw/croutine/croutine.h"
#include "rcmw/scheduler/scheduler.h"
#include "rcmw/config/classic_conf.h"

namespace hnu       {
namespace rcmw      {
namespace scheduler {

using hnu::rcmw::croutine::CRoutine;
using hnu::rcmw::config::ClassicConf;
using hnu::rcmw::config::ClassicTask;

class SchedulerClassic : public Scheduler {
public:
    bool RemoveCRoutine(uint64_t crid) override;
    bool RemoveTask(const std::string& name) override;
    bool DispatchTask(const std::shared_ptr<CRoutine>& cr) override;
private:
    friend Scheduler* Instance();
    SchedulerClassic();
    void CreateProcessor();
    bool NotifyProcessor(uint64_t crid) override;

    std::unordered_map<std::string, ClassicTask> cr_confs_;
    ClassicConf classic_conf_;
};

} // scheduler
} // rcmw
} // hnu

#endif
