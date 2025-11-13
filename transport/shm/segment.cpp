/**
 * @brief 共享内存操作基类实现
 * @date 2025.11.13
 */

#include "segment.h"
#include "rcmw/logger/log.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

Segment::Segment(uint64_t channel_id) : 
    init_(false), 
    conf_(), 
    channel_id_(channel_id),
    state_(nullptr), 
    blocks_(nullptr),
    managed_shm_(nullptr),
    block_buf_lock_(),
    block_buf_addrs_() {}

bool Segment::AcquireBlockToWrite(std::size_t msg_size, WritableBlock* writable_block) {
    /* 确认内存是否已经创建？ */
    RETURN_VAL_IF_NULL(writable_block, false);
    if(!init_ && !OpenOrCreate()) {
        AERROR << "create shm failed, can't write now.";
        return false;
    }

    /* 确认是否需要重映射 */
    bool result = true;
    if(state_->need_remap()) {
        result = Remap();
    }

    /* 内存大小检查 */
    if(msg_size > conf_.ceiling_msg_size()) {
        AERROR << "msg_size: " << msg_size
            << "larger than current shm_buffer_size: "
            << conf_.ceiling_msg_size() << ", need recreate.";
        result = Recreate(msg_size);
    }
    /* 判断内存是否可用 */
    if(!result) {
        AERROR << "segment update failed.";
        return false;
    }

    /* 取指定内存 */
    uint32_t index = GetNextWritableBlockIndex();
    writable_block->index = index;
    writable_block->block = &blocks_[index];
    writable_block->buf - block_buf_addrs_[index];
    return true;
}

void Segment::ReleaseWrittenBlock(const WritableBlock& writable_block) {
    auto index = writable_block.index;
    if(index >= conf_.block_num()) return;
    blocks_[index].ReleaseWriteLock();
}

bool Segment::AcquireBlockToRead(ReadableBlock* readable_block) {
    /* 共享内存区检查 */
    RETURN_VAL_IF_NULL(readable_block, false);
    if(!init_ && !OpenOrCreate()) {
        AERROR << "create shm failed, can't write now.";
        return false;
    }

    auto index = readable_block->index;
    if(index >= conf_.block_num()) {
        AERROR << "invalid block_index[" << index << "].";
        return false;
    }

    bool result = true;
    if(state_->need_remap()) result = Remap();

    if(!result) {
        AERROR << "segment update failed.";
        return false;
    }

    /* 取锁 */
    if(!blocks_[index].TryLockForRead()) return false;
    readable_block->block = blocks_ + index;
    readable_block->buf = block_buf_addrs_[index];
    return true;
}

void Segment::ReleaseReadableBlock(const ReadableBlock* readable_block) {
    auto index = readable_block->index;
    if(index >= conf_.block_buf_size()) return;
    blocks_[index].ReleaseReadLock();
}

bool Segment::Destory() {
    if(!init_) return true;
    init_ = false;

    try {
        state_->DecreaseReferenceCount();
        uint32_t reference_counts = state_->reference_count();
        if(reference_counts == 0) return Remove();
    }
    catch(...) {
        AERROR << "exception.";
        return false;
    }
    ADEBUG << "destroy.";
    return true;
}

bool Segment::Remap() {
    init_ = false;
    ADEBUG << "before reset.";
    Reset();
    ADEBUG << "after reset.";
    return OpenOnly();
}

bool Segment::Recreate(const uint64_t& msg_size) {
    init_ = false;
    state_->set_need_remap(true);
    Reset();
    Remove();
    conf_.Update(msg_size);
    return OpenOrCreate();
}

uint32_t Segment::GetNextWritableBlockIndex() {
    const auto block_num = conf_.block_num();
    while (true)
    {
        uint32_t try_idx = state_->FetchAddSeq(1) % block_num;
        if(blocks_[try_idx].TryLockForWrite()) return try_idx;
    }
    return 0;
}

} // transport
} // rcmw
} // hnu
