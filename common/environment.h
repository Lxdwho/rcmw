/**
 * @brief 环境变量的获取与设置
 * @date 2025.11.11
 */

#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <string>
#include <iostream>
#include "rcmw/logger/log.h"

namespace hnu    {
namespace rcmw   {
namespace common {

/* 获取环境变量 */
inline std::string GetEnv(const std::string& var_name, 
                        const std::string& default_value = " \"\" ") {
    const char* var = std::getenv(var_name.c_str());
    if(var == nullptr) {
        AWARN << "Environment variable [" << var_name 
                << "] not set, fallback to " << default_value;
        return default_value;
    }
    return std::string(var);
}

/* 设置工作空间路径 */
inline const std::string WorkRoot() {
    std::string work_root = GetEnv("RCMW_PATH");
    if(work_root.empty()) {
        work_root = "/rcmw";
    }
    return work_root;
}

}
}
}

#endif
