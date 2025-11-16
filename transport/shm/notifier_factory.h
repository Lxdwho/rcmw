/**
 * @brief 
 * @date 2025.11.15
 */

#ifndef _NOTIFIER_FACTORY_H_
#define _NOTIFIER_FACTORY_H_

#include "notifier_base.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

class NotifierFactory{
public:
    static NotifierPtr CreateNotifier();
private:
    static NotifierPtr CreateConditionNotifier();
    static NotifierPtr CreateMulticastNotifier();
};

} // transport
} // rcmw
} // hnu

#endif
