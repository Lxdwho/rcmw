/**
 * @brief 文件路径相关操作
 * @date 2025.11.17
 */

#include <string>
#include <sys/stat.h>

namespace hnu    {
namespace rcmw   {
namespace common {

std::string GetAbsolutePath(const std::string& prefix, 
                            const std::string& relative_path) {
    if(relative_path.empty()) return prefix;
    if(prefix.empty() || relative_path.front() == '/') return relative_path;
    if(prefix.back() == '/') return prefix + relative_path;
    return prefix + "/" + relative_path;
}

bool PathExist(const std::string& path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0;
}

std::string GetFileName(const std::string& path, 
                        const bool remove_extension = false) {
    std::string::size_type start = path.rfind('/');
    if(start == std::string::npos) start = 0;
    else ++start;

    std::string::size_type end = std::string::npos;
    /* 如果要移除拓展名 */
    if(remove_extension) {
        end = path.rfind('.');
        if(end != std::string::npos && end < start) end = std::string::npos;
    }
    const auto len = (end != std::string::npos) ? end - start : end;
    return path.substr(start, len);
}

} // common
} // rcmw
} // hnu
