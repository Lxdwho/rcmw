/**
 * @brief 
 * @date 2025.11.18
 */

#ifndef _COMPONENT_CONF_H_
#define _COMPONENT_CONF_H_

#include <string>

namespace hnu    {
namespace rcmw   {
namespace config {

struct ComponentConfig
{
    std::string name;
    std::string config_file_path;
    std::string flag_file_path;
};

struct TimerComponentConfig {
    std::string name;
    std::string config_file_path;
    std::string flag_file_path;
    uint32_t interval;
};

} // config
} // rcmw
} // hnu

#endif
