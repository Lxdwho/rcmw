/**
 * @brief 调度器单例工厂
 * @date 2026.01.04
 */

#include "rcmw/scheduler/scheduler_factory.h"
#include "rcmw/config/conf_parse.h"

namespace hnu       {
namespace rcmw      {
namespace scheduler {

using hnu::rcmw::common::GetAbsolutePath;
using hnu::rcmw::common::GlobalData;
using hnu::rcmw::common::PathExist;
using hnu::rcmw::common::WorkRoot;
using hnu::rcmw::config::GetRcmwConfFromFile;
using hnu::rcmw::config::RcmwConfig;

namespace {
    std::atomic<Scheduler*> instance = {nullptr};
    std::mutex mutex;
}

Scheduler* Instance() {
    Scheduler* obj = instance.load(std::memory_order_acquire);
    if(obj == nullptr) {
        std::lock_guard<std::mutex> lock(mutex);
        obj = instance.load(std::memory_order_acquire);
        if(obj == nullptr) {
            std::string policy;
            auto config_file = std::string("conf/") + 
                GlobalData::Instance()->ProcessGroup() + ".conf";
            RcmwConfig config;
            if(PathExist(config_file) && GetRcmwConfFromFile(config_file, &config)) {
                AINFO << "Scheduler conf " << config_file << "found and used.";
                policy = config.scheduler_conf.policy;
            }
            else {
                auto config_path = GetAbsolutePath(WorkRoot(), config_file);
                if(PathExist(config_path) && GetRcmwConfFromFile(config_path, &config)) {
                    AINFO << "Scheduler conf " << config_path << "found and used.";
                    policy = config.scheduler_conf.policy;
                }
                else {
                    policy = "classic";
                    AWARN << "No scheduler conf " << config_path << " found, use default.";
                }
            }

            if(!policy.compare("classic")) {
                obj = new SchedulerClassic();
                AINFO << "new SchedulerClassic() ";
            }
            else if(!policy.compare("choreography")) {
                obj = new SchedulerClassic();
                AINFO << "choreography is not ready new SchedulerClassic() ";
            }
            else {
                AWARN << "Invalid scheduler policy: " << policy;
                obj = new SchedulerClassic();
            }
            instance.store(obj, std::memory_order_release);
        }
    }
    return obj;
}

void CleanUp() {
    Scheduler* obj = instance.load();
    if(obj != nullptr) {
        obj->Shutdown();
        /* 指针置空？ */
    }
}

} // scheduler
} // rcmw
} // hnu
