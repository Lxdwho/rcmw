/**
 * @brief 基于共享内存的消息通知？
 * @date 2025.11.15
 */

#ifndef _CONDITION_NOTIFIER_H_
#define _CONDITION_NOTIFIER_H_

#include "notifier_base.h"
#include <atomic>
#include "rcmw/common/macros.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

const uint32_t KBufLength = 4096;
class ConditionNotifier : public NotifierBase {
    struct Indicator
    {
        std::atomic<uint64_t> next_seq = { 0 };
        ReadableInfo infos[KBufLength];
        uint64_t seqs[KBufLength] = { 0 };
    };
public:
    virtual ~ConditionNotifier();
    void Shutdown() override;
    bool Notify(const ReadableInfo& info) override;
    bool Listen(int timeout_ms, ReadableInfo* info) override;
    static const char* Type() { return "contion"; }
private:
    bool Init();
    bool OpenOrCreate();
    bool OpenOnly();
    bool Remove();
    void Reset();

    key_t key_ = 0;
    void* managed_shm_ = nullptr;
    size_t shm_size_ = 0;
    Indicator* indicator_ = nullptr;
    uint64_t next_seq_ = 0;
    std::atomic<bool> is_shutdown_ = { false };
    DECLARE_SINGLETON(ConditionNotifier)
};

} // transport
} // rcmw
} // hnu

#endif
