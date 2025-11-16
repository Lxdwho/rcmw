/**
 * @brief 消息通知
 * @date 2025.11.15
 */

#ifndef _NOTIFIER_BASE_H_
#define _NOTIFIER_BASE_H_

#include "rcmw/transport/shm/readable_info.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

class NotifierBase;
using NotifierPtr = NotifierBase*;
class NotifierBase {
public:
    virtual ~NotifierBase() = default;
    virtual void Shutdown() = 0;
    virtual bool Notify(const ReadableInfo& info) = 0;
    virtual bool Listen(int timeout_ms, ReadableInfo* info) = 0;
};

} // transport
} // rcmw
} // hnu

#endif
