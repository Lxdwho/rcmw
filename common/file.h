/**
 * @brief 文件路径相关操作
 * @date 2025.11.17
 */

#ifndef _COMMON_FILE_H_
#define _COMMON_FILE_H_

#include <string>

namespace hnu    {
namespace rcmw   {
namespace common {

std::string GetAbsolutePath(const std::string& prefix, 
                            const std::string& relative_path);
                            
bool PathExist(const std::string& path);

std::string GetFileName(const std::string& path, 
                        const bool remove_extension = false);
}
}
}

#endif
