/**
 * @brief 基于shm的数据分发器
 * @date 2025.12.10
 */

#include "shm_dispatcher.h"
#include "rcmw/common/global_data.h"
#include "rcmw/common/util.h"
#include "rcmw/transport/shm/notifier_factory.h"
#include "rcmw/transport/shm/segment_factory.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

using common::GlobalData;

ShmDispatcher::ShmDispatcher() : host_id_(0) { Init(); }
ShmDispatcher::~ShmDispatcher() { Shutdown(); }

bool ShmDispatcher::Init() {
    host_id_ = common::Hash(GlobalData::Instance()->HostIp());
    notifier_ = NotifierFactory::CreateNotifier();
    thread_ = std::thread(&ShmDispatcher::ThreadFunc, this);
    return true;
}

void ShmDispatcher::OnMessage(uint64_t channel_id,
                                const std::shared_ptr<ReadableBlock>& rb,
                                const MessageInfo& msg_info) {
    if(is_shutdown_.load()) return;
    ListenerHandlerBasePtr* handler_base = nullptr;
    if(msg_listeners_.Get(channel_id, &handler_base)) {
        auto handler = std::dynamic_pointer_cast<ListenerHandler<ReadableBlock>>(*handler_base);
        handler->Run(rb, msg_info);
    }
    else {
        AERROR << "Cannot find " << GlobalData::GetChannelById(channel_id) << "'s handler.";
    }
}

void ShmDispatcher::AddSegment(const RoleAttributes& self_attr) {
    uint64_t channel_id = self_attr.channel_id;
    WriteLockGuard<AtomicRWLock> lock(segments_lock_);
    if(segments_.count(channel_id) > 0) return;
    auto segment = SegmentFactory::CreateSegment(channel_id);
    segments_[channel_id] = segment;
    previous_indexs_[channel_id] = UINT32_MAX;
}

void ShmDispatcher::ThreadFunc() {
    ReadableInfo readable_info;
    while (!is_shutdown_.load())
    {
        if(!notifier_->Listen(100, &readable_info)) continue;
        if(readable_info.host_id() != host_id_) {
            ADEBUG << "shm readable info from other host. " << host_id_
                   << " " << readable_info.host_id();
            continue;
        }
        uint64_t channel_id = readable_info.channel_id();
        uint64_t block_index = readable_info.block_index();

        {
            ReadLockGuard<AtomicRWLock> lg(segments_lock_);
            if(segments_.count(channel_id) == 0) continue;
            if(previous_indexs_.count(channel_id) == 0)
                previous_indexs_[channel_id] == UINT32_MAX;
            uint32_t& previous_index = previous_indexs_[channel_id];
            if(block_index != 0 && previous_index != UINT32_MAX) {
                if(block_index == previous_index)
                    ADEBUG << "Receive SAME index " << block_index 
                           << "of channel " << channel_id;
                else if(block_index < previous_index)
                    ADEBUG << "Receive PREVIOUS message. last: " << previous_index 
                           << ", now: " << block_index;
                else if(block_index - previous_index > 1)
                    ADEBUG << "Receive JUMP message. last: " << previous_index
                           << ", now: " << block_index;
            }
            previous_index = block_index;
            ReadMessage(channel_id, block_index);
        }
    }
}

void ShmDispatcher::ReadMessage(uint64_t channel_id, uint32_t block_index) {
    ADEBUG << "Reading sharedmem message: " 
           << GlobalData::GetChannelById(channel_id)
           << " index: " << block_index;
    auto rb = std::make_shared<ReadableBlock>();
    rb->index = block_index;
    if(!segments_[channel_id]->AcquireBlockToRead(rb.get())) {
        AWARN << "fail to acquire block, channel: "
              << GlobalData::GetChannelById(channel_id)
              << " index: " << block_index;
    }
    MessageInfo msg_info;
    const char* msg_info_addr = reinterpret_cast<char*>(rb->buf) + rb->block->msg_size();
    std::memcpy((void*)msg_info.sender_id().data(), (void*)msg_info_addr, ID_SIZE);
    std::memcpy((void*)msg_info.spaer_id().data(), (void*)(msg_info_addr + ID_SIZE), ID_SIZE);
    msg_info.set_seq_num(*(reinterpret_cast<uint64_t*>(const_cast<char*>(msg_info_addr + 2*ID_SIZE))));
    OnMessage(channel_id, rb, msg_info);
    segments_[channel_id]->ReleaseReadableBlock(*rb);
}

void ShmDispatcher::Shutdown() {
    if(is_shutdown_.exchange(true)) return;
    if(thread_.joinable()) thread_.join();
    
    {
        ReadLockGuard<AtomicRWLock> lock(segments_lock_);
        segments_.clear();
    }
}

} // transport
} // rcmw
} // hnu
