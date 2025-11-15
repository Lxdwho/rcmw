/**
 * @brief 待看
 * @date 2025.11.14
 */

#ifndef _CLASSIC_CONF_H_
#define _CLASSIC_CONF_H_

#include <string>
#include <vector>
#include <cstdint>

namespace hnu    {
namespace rcmw   {
namespace config {

struct ClassicTask {
    std::string name;
    uint32_t prio = 0;
    std::string group_name;
};

struct SchedGroup
{
    std::string name;
    uint32_t processor_num = 0;
    std::string affinity;
    std::string cpuset;
    std::string processor_policy;
    int32_t processor_prio = 0;
    std::vector<ClassicTask> tasks;
};

struct ClassicConf {
    std::vector<SchedGroup> groups;
};

}
}
}

#endif
