/**
 * @brief 从文件加载配置操作
 * @date 2025.11.17
 */

#ifndef _COMMON_CONF_PARSE_H_
#define _COMMON_CONF_PARSE_H_

#include <string>
#include "rcmw_conf.h"

namespace hnu    {
namespace rcmw   {
namespace config {

bool GetRcmwConfFromFile(const std::string& file_path, 
                            RcmwConfig* config);

}
}
}

#endif
