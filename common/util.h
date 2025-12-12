/**
 * @brief 
 * @date 2025.11.12
 */

#ifndef _COMMON_UTIL_H_
#define _COMMON_UTIL_H_

#include <string>
#include <functional>

namespace hnu    {
namespace rcmw   {
namespace common {

/**
 * @brief 获取一个字符串的哈希值
 * std::hash<std::string>{}, 表示创建一个std::hash<std::string>的临时对象
 * {}中是初始化列表，同时std::hash<std::string>内部重载了()，用于获取哈希值。
 */
inline std::size_t Hash(const std::string& key) {
    return std::hash<std::string>{}(key);
}

}
}
}

#endif
