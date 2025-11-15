/**
 * @brief 共享内存状态类实现
 * @date 2025.11.12
 */

#include "state.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

State::State(const uint64_t& ceiling_msg_size) : 
    ceiling_msg_size_(ceiling_msg_size) {}

State::~State() {}

} // transport
} // rcmw
} // hnu
