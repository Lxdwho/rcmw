/**
 * @brief 数据分发器
 * @date 2025.12.10
 */

#include "dispatcher.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

Dispatcher::Dispatcher() : is_shutdown_(false) {}
Dispatcher::~Dispatcher() { Shutdown(); }

void Dispatcher::Shutdown() {
    is_shutdown_.store(true);
    ADEBUG << "Shutdown";
}

bool Dispatcher::HasChannel(uint64_t channel_id) {
    return msg_listeners_.Has(channel_id);
}

} // transport
} // rcmw
} // hnu
