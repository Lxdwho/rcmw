/**
 * @brief rcmw运行状态
 * @date 2025.12.26
 */

#include "rcmw/state.h"
#include <atomic>

namespace hnu   {
namespace rcmw  {

namespace {
    std::atomic<State> g_rcmw_state;
}

State GetState() { return g_rcmw_state.load(); }

void SetState(const State& state) { g_rcmw_state.store(state); }

} // rcmw
} // hnu
