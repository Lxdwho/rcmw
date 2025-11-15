/**
 * @brief 基于posix的共享内存实现
 * @date 2025.11.14
 */

#include "posix_segment.h"
#include "rcmw/logger/log.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>

namespace hnu       {
namespace rcmw      {
namespace transport {

PosixSegment::PosixSegment(uint64_t channel_name) : Segment(channel_name) {
    shm_name_ = std::to_string(channel_name);
}

PosixSegment::~PosixSegment() { Destory(); }

void PosixSegment::Reset() {
    state_ = nullptr;
    blocks_ = nullptr;
    {
        std::lock_guard<std::mutex> lock(block_buf_lock_);
        block_buf_addrs_.clear();
    }
    if(managed_shm_ != nullptr) {
        munmap(managed_shm_, conf_.managed_shm_size());
        managed_shm_ = nullptr;
        return;
    }
}

bool PosixSegment::Remove() {
    if(shm_unlink(shm_name_.c_str()) < 0) {
        AERROR << "shm_unlink failed: " << strerror(errno);
        return false;
    }
    return true;
}

bool PosixSegment::OpenOnly() {
    if(init_) return true;

    int fd = shm_open(shm_name_.c_str(), O_RDWR, 0644);
    if(fd == -1) {
        AERROR << "get shm failed: " << strerror(errno);
        close(fd);
        return false;
    }

    struct stat file_attr;
    if(fstat(fd, &file_attr) < 0) {
        AERROR << "fstat failed: " << strerror(errno);
        close(fd);
        return false;
    }

    managed_shm_ = mmap(nullptr, file_attr.st_size, 
                    PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(managed_shm_ == MAP_FAILED) {
        AERROR << "attach shm failed: " << strerror(errno);
        close(fd);
        return false;
    }
    close(fd);
    state_ = reinterpret_cast<State*>(managed_shm_);
    if(state_ == nullptr) {
        AERROR << "get state failed.";
        munmap(managed_shm_, file_attr.st_size);
        managed_shm_ = nullptr;
        return false;
    }
    
    conf_.Update(state_->ceiling_msg_size());
    blocks_ = reinterpret_cast<Block*>(static_cast<char*>(managed_shm_) + sizeof(State));
    if(blocks_ == nullptr) {
        AERROR << "get blocks failed.";
        munmap(managed_shm_, conf_.managed_shm_size());
        managed_shm_ = nullptr;
        return false;
    }

    uint32_t i=0;
    for(; i < conf_.block_num(); i++) {
        uint8_t* addr = reinterpret_cast<uint8_t*>(
            static_cast<char*>(managed_shm_) + conf_.block_num() * sizeof(Block) + 
            i * conf_.block_buf_size());
    }
    if(i != conf_.block_num()) {
        AERROR << "open only failed.";
        state_->~State();
        state_ = nullptr;
        blocks_ = nullptr;
        {
            std::lock_guard<std::mutex> lock(block_buf_lock_);
            block_buf_addrs_.clear();
        }
        munmap(managed_shm_, conf_.managed_shm_size());
        managed_shm_ = nullptr;
        shm_unlink(shm_name_.c_str());
        return false;
    }
    state_->IncreaseReferenceCount();
    init_ = true;
    AINFO << "open only true.";
    return true;
}

bool PosixSegment::OpenOrCreate() {
    if(init_) return true;
    int fd = shm_open(shm_name_.c_str(), O_RDWR | O_CREAT | O_EXCL, 0644);
    if(fd < 0) {
        if(EEXIST == errno) {
            AERROR << "shm already exist, open only.";
            return OpenOnly();
        }
        else {
            AERROR << "create shm failed: " << strerror(errno);
            return false;
        }
    }

    if(ftruncate(fd, conf_.managed_shm_size()) < 0) {
        AERROR << "ftruncate failed: " << strerror(errno);
        close(fd);
        return false;
    }

    managed_shm_ = mmap(nullptr, conf_.managed_shm_size(), PROT_READ | PROT_WRITE, 
                        MAP_SHARED, fd, 0);
    if(managed_shm_ == MAP_FAILED) {
        AERROR << "attach shm failed: " << strerror(errno);
        close(fd);
        shm_unlink(shm_name_.c_str());
        return false;
    }

    close(fd);

    state_ = new (managed_shm_) State(conf_.ceiling_msg_size());
    if(state_ == nullptr) {
        AERROR << "create state_ failed.";
        munmap(managed_shm_, conf_.managed_shm_size());
        managed_shm_ = nullptr;
        shm_unlink(shm_name_.c_str());
        return false;
    }

    blocks_ = new(static_cast<char*>(managed_shm_) + sizeof(State)) Block[conf_.block_num()];
    if(blocks_ == nullptr) {
        AERROR <<  "create blocks failed.";
        state_->~State();
        state_ = nullptr;
        munmap(managed_shm_, conf_.managed_shm_size());
        managed_shm_ = nullptr;
        shm_unlink(shm_name_.c_str());
        return false;
    }

    uint32_t i=0;
    for(; i < conf_.block_num(); i++) {
        uint8_t* addr = new (static_cast<char*>(managed_shm_) + sizeof(State) + 
                conf_.block_num() * sizeof(Block) + i * conf_.block_buf_size())
                uint8_t[conf_.block_buf_size()];
        if(addr == nullptr) break;
        std::lock_guard<std::mutex> lock(block_buf_lock_);
        block_buf_addrs_[i] = addr;
    }
    if(i != conf_.block_num()) {
        AERROR << "create block buf failed.";
        state_->~State();
        state_ = nullptr;
        blocks_ = nullptr;
        {
            std::lock_guard<std::mutex> lock(block_buf_lock_);
            block_buf_addrs_.clear();
        }
        munmap(managed_shm_, conf_.managed_shm_size());
        managed_shm_ = nullptr;
        shm_unlink(shm_name_.c_str());
        return false;
    }

    state_->IncreaseReferenceCount();
    init_ = true;
    return true;
}

} // transport
} // rcmw
} // hnu
