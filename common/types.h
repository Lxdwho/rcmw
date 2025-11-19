/**
 * @brief 
 * @date 2025.11.18
 */

#ifndef _COMMON_TYPES_H_
#define _COMMON_TYPES_H_

#include <cstdint>

namespace hnu       {
namespace rcmw      {

class NullType {};

/**
 * @brief Describe relation between nodes, 
 *        writers/readers...
 */
enum Relation : std::uint8_t {
    NO_RELATION = 0,
    DIFF_HOST,          // different host
    DIFF_PROC,          // same host, but different process
    SAME_PROC,          // same process
};

} // rcmw
} // hnu

#endif
