/**
 * @brief 基于System v 的segment子类
 * @date 2025.11.13
 */

#include "xsi_segment.h"
#include "rcmw/logger/log.h"
#include <sys/shm.h>
#include <sys/ipc.h>

namespace hnu       {
namespace rcmw      {
namespace transport {

XsiSegment::XsiSegment(uint64_t channel_id) : Segment(channel_id) {
    key_ = static_cast<key_t>(channel_id);
}

XsiSegment::~XsiSegment() { Destory(); }

void XsiSegment::Reset() {
    state_ = nullptr;
    blocks_ = nullptr;
    {
        std::lock_guard<std::mutex> _g(block_buf_lock_);
        block_buf_addrs_.clear();
    }
    if(managed_shm_ != nullptr) {
        shmdt(managed_shm_);
        managed_shm_ = nullptr;
        return;
    }
}

bool XsiSegment::Remove() {
    int shmid = shmget(key_, 0, 0644);
    if(shmid == -1 || shmctl(shmid, IPC_RMID, 0) == -1) {
        AERROR << "remove shm failed, error code: " << strerror(errno);
        return false;
    }
    ADEBUG << "remove success.";
    return true;
}

bool XsiSegment::OpenOnly() {
    if(init_) return true;

    /* 拿到一片共享内存？ */
    int shmid = shmget(key_, 0, 0644);
    if(shmid == -1) {
        AERROR << "attach shm failed, error: " << strerror(errno);
        return false;
    }

    /* 映射到虚拟内存 managed_shm_ */
    managed_shm_ = shmat(shmid, nullptr, 0);
    if(managed_shm_ == reinterpret_cast<void*>(-1)) {
        AERROR << "attach shm failed, error: " << strerror(errno);
        return false;
    }

    /* 获取 state_ */
    state_ = reinterpret_cast<State*>(managed_shm_);
    if(state_ == nullptr) {
        AERROR << "get state failed.";
        shmdt(managed_shm_);
        managed_shm_ = nullptr;
        return false;
    }

    /* 更新配置大小 */
    conf_.Update(state_->ceiling_msg_size());

    blocks_ = reinterpret_cast<Block*>(static_cast<char*>(managed_shm_) + sizeof(State));
    if(blocks_ == nullptr) {
        AERROR << "get blocks failed.";
        state_ = nullptr;
        shmdt(managed_shm_);
        managed_shm_ = nullptr;
        return false;
    }

    uint32_t i=0;
    for(;i < conf_.block_num(); i++) {
        uint8_t* addr = reinterpret_cast<uint8_t*>(
            static_cast<char*>(managed_shm_) + sizeof(State) +
            conf_.block_num() * sizeof(Block) + i * conf_.block_buf_size());
        if(addr == nullptr) break;
        std::lock_guard<std::mutex> _g(block_buf_lock_);
        block_buf_addrs_[i] = addr;
    }

    if(i != conf_.block_num()) {
        AERROR << "open only failed.";
        state_->~State();
        state_ = nullptr;
        blocks_ = nullptr;
        {
            std::lock_guard<std::mutex> _g(block_buf_lock_);
            block_buf_addrs_.clear();
        }
        shmdt(managed_shm_);
        managed_shm_ = nullptr;
        shmctl(shmid, IPC_RMID, 0);
        return false;
    }

    state_->IncreaseReferenceCount();
    init_ = true;
    ADEBUG << "open only true.";
    return true;
}

bool XsiSegment::OpenOrCreate() {
    if(init_) return true;

    int retry = 0, shmid = 0;
    while(retry < 2) {
        shmid = shmget(key_, conf_.managed_shm_size(), 0644 | IPC_CREAT | IPC_EXCL);
        if(shmid != -1) break;
        if(EINVAL == errno) {
            AINFO << "need larger space, recreate.";
            Reset();
            Remove();
            ++retry;
        }
        else if(EEXIST == errno) {
            ADEBUG << "shm already exist, open only.";
            return OpenOnly();
        }
        else break;
    }

    if(shmid == -1) {
        AERROR << "create shm failed, error code: " << strerror(errno);
        return false;
    }

    /* 映射共享内存 */
    managed_shm_ = shmat(shmid, nullptr, 0);
    if(managed_shm_ == reinterpret_cast<void*>(-1)) {
        AERROR << "attach shm failed, error: " << strerror(errno);
        shmctl(shmid, IPC_RMID, 0);
        return false;
    }

    state_ = new (managed_shm_) State(conf_.ceiling_msg_size());
    if(state_ == nullptr) {
        AERROR << "creat state failed.";
        shmdt(managed_shm_);
        managed_shm_ = nullptr;
        shmctl(shmid, IPC_RMID, 0);
        return false;
    }

    conf_.Update(state_->ceiling_msg_size());
    
    blocks_ = new (static_cast<char*>(managed_shm_) + sizeof(State)) Block[conf_.block_num()];
    if(blocks_ == nullptr) {
        AERROR << "create blocks failed.";
        state_->~State();
        state_ = nullptr;
        shmdt(managed_shm_);
        managed_shm_ = nullptr;
        shmctl(shmid, IPC_RMID, 0);
        return false;
    }

    uint32_t i=0;
    for(; i < conf_.block_num(); ++i) {
        uint8_t* addr = new (static_cast<char*>(managed_shm_) + sizeof(State) + 
                            conf_.block_num() * sizeof(Block) + i * conf_.block_buf_size())
                            uint8_t[conf_.block_buf_size()];
        std::lock_guard<std::mutex> _g(block_buf_lock_);
        block_buf_addrs_[i] = addr;
    }

    if(i != conf_.block_num()) {
        AERROR << "create block buf failed.";
        state_->~State();
        state_ = nullptr;
        blocks_ = nullptr;
        {
            std::lock_guard<std::mutex> _g(block_buf_lock_);
            block_buf_addrs_.clear();
        }
        shmdt(managed_shm_);
        managed_shm_ = nullptr;
        shmctl(shmid, IPC_RMID, 0);
        return false;
    }

    state_->IncreaseReferenceCount();
    init_ = true;
    ADEBUG << "open or create true.";
    return true;
}

} // transport
} // rcmw
} // hnu
