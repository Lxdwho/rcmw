/**
 * @brief 
 * @date 2025.11.15
 */

#include "notifier_factory.h"
#include "condition_notifier.h"
#include "multicast_notifier.h"
#include <string>
#include "rcmw/logger/log.h"

namespace hnu       {
namespace rcmw      {
namespace transport {


auto NotifierFactory::CreateNotifier() -> NotifierPtr {
    std::string notifier_type(ConditionNotifier::Type());
    if(notifier_type == MulticastNotifier::Type()) {
        return CreateMulticastNotifier();
    }
    else if(notifier_type == ConditionNotifier::Type()) {
        return CreateConditionNotifier();
    }
    AINFO << "unknow notifier, used default notifier: " << notifier_type;
    return CreateConditionNotifier();
}

auto NotifierFactory::CreateConditionNotifier() -> NotifierPtr {
    return ConditionNotifier::Instance();
}

auto NotifierFactory::CreateMulticastNotifier() -> NotifierPtr {
    return MulticastNotifier::Instance();
}

} // transport
} // rcmw
} // hnu
