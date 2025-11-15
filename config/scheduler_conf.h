/**
 * @brief 待看
 * @date 2025.11.14
 */

#ifndef _SCHEDULER_CONF_H_
#define _SCHEDULER_CONF_H_

#include "rcmw/config/classic_conf.h"
#include <string>
#include <vector>
#include <cstdint>

namespace hnu    {
namespace rcmw   {
namespace config {

struct InnerThread {
    std::string name;
    std::string cpuset;
    std::string policy;
    uint32_t prio = 0;
};

struct SchedulerConf {
    std::string policy;
    uint32_t routine_num = 0;
    uint32_t default_proc_num = 0;
    std::string process_level_cpuset;
    std::vector<InnerThread> threads;
    ClassicConf classic_conf;
};

}
}
}

#endif
