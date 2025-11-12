/**
 * @brief 共享内存块锁机制？
 * @date 2025.11.12
 */

#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <atomic>

namespace hnu       {
namespace rcmw      {
namespace transport {

class Block {
public:
    Block();
    virtual ~Block();

    uint64_t msg_size() const { return msg_size_; }
    void set_msg_size(uint64_t msg_size) { msg_size_ = msg_size; }
    uint64_t msg_info_size() const { return msg_info_size_; }
    void set_msg_info_size(uint64_t msg_info_size) { msg_info_size_ = msg_info_size; }

    static const int32_t KRWLockFree;
    static const int32_t KWriteExclusive;
    static const int32_t KMaxTryLockTimes;

private:
    bool TryLockForWrite();
    bool TryLockForRead();
    void ReleaseWriteLock();
    void ReleaseReadLock();

    std::atomic<int32_t> lock_num_ = { 0 };
    uint64_t msg_size_;
    uint64_t msg_info_size_;
};

} // transport
} // rcmw
} // hnu

#endif
