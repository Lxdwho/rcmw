/**
 * @brief 基于网络组播的消息通知？
 * @date 2025.11.15
 */

#ifndef _MULTICAST_NOTIFIER_H_
#define _MULTICAST_NOTIFIER_H_

#include <netinet/in.h>
#include <atomic>
#include "notifier_base.h"
#include "rcmw/common/macros.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

class MulticastNotifier : public NotifierBase {
public:
    virtual ~MulticastNotifier();
    void Shutdown() override;
    bool Notify(const ReadableInfo& info) override;
    bool Listen(int timesout_ms, ReadableInfo* info) override;
    static const char* Type() { return "multicast"; }
private:
    bool Init();
    
    int notify_fd_ = -1;
    struct sockaddr_in notify_addr_;

    int listen_fd_ = -1;
    struct sockaddr_in listen_addr_;

    std::atomic<bool> is_shutdown_ = { false };
    DECLARE_SINGLETON(MulticastNotifier)
};

} // transport
} // rcmw
} // hnu

#endif
