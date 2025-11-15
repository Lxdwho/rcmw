/**
 * @brief 
 * @date 2025.11.14
 */

#ifndef _RCMW_CONF_H_
#define _RCMW_CONF_H_

#include "scheduler_conf.h"
#include "transport_conf.h"
#include "nlohmann/json.hpp"

namespace hnu    {
namespace rcmw   {
namespace config {

struct RcmwConfig {
    SchedulerConf scheduler_conf;
    TransprotConf transprot_conf;
};

void from_json(const nlohmann::json& j, InnerThread& thread);
void from_json(const nlohmann::json& j, ClassicTask& task);
void from_json(const nlohmann::json& j, SchedGroup& group);
void from_json(const nlohmann::json& j, ClassicConf& classic_conf);
void from_json(const nlohmann::json& j, SchedulerConf& Scheduler_conf);
void from_json(const nlohmann::json& j, RcmwConfig& rcmw_config);

}
}
}

#endif
