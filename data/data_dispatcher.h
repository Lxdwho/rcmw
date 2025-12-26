/**
 * @brief 数据分发器？
 * @date 2025.12.26
 */

#ifndef _RCMW_DATA_DATA_DISPATCHER_H_
#define _RCMW_DATA_DATA_DISPATCHER_H_

#include <memory>
#include <mutex>
#include <vector>
#include "rcmw/data/buffer/channel_buffer.h"
#include "rcmw/data/data_notifier.h"
#include "rcmw/common/macros.h"
#include "rcmw/logger/log.h"
#include "rcmw/time/time.h"
#include "rcmw/state.h"

namespace hnu   {
namespace rcmw  {
namespace data  {

using hnu::rcmw::Time;
using hnu::rcmw::base::AtomicHashMap;

/**
 * @brief 数据分发器类，单例
 */
template<typename T>
class DataDispatcher {
public:
    using BufferVector = std::vector<std::weak_ptr<CacheBuffer<std::shared_ptr<T>>>>;
    ~DataDispatcher() {}
    void AddBuffer(const ChannelBuffer<T>& channel_buffer);
    bool Dispatch(const uint64_t channel_id, const std::shared_ptr<T>& msg);
private:
    DataNotifier* notifier_ = DataNotifier::Instance();
    std::mutex buffer_map_mutex_;
    AtomicHashMap<uint64_t, BufferVector> buffers_map_;
    DECLARE_SINGLETON(DataDispatcher);
};

template<typename T>
inline DataDispatcher<T>::DataDispatcher() {}

/**
 * @brief 添加buffer
 * @param channel_buffer: 添加的对象
 */
template<typename T>
void DataDispatcher<T>::AddBuffer(const ChannelBuffer<T>& channel_buffer) {
    std::lock_guard<std::mutex> lock(buffer_map_mutex_);
    auto buffer = channel_buffer.Buffer();
    BufferVector* buffers = nullptr;
    if(buffers_map_.Get(channel_buffer.channel_id(), buffers)) {
        buffers->emplace_back(buffer);
    }
    else {
        BufferVector new_buffers = {buffer};
        buffers_map_.Set(channel_buffer.channel_id(), new_buffers);
    }
}

/**
 * @brief 添加buffer
 * @param channel_id: 分发的通道id
 * @param msg: 分发的消息
 */
template<typename T>
bool DataDispatcher<T>::Dispatch(const uint64_t channel_id, const std::shared_ptr<T>& msg) {
    BufferVector* buffers = nullptr;
    if(hnu::rcmw::IsShutdown()) return false;
    if(buffers_map_.Get(channel_id, &buffers)) {
        for(auto& buffer_wptr : *buffers) {
            if(auto buffer = buffer_wptr.lock()) {
                std::lock_guard<std::mutex> lock(buffer->Mutex());
                buffer->Fill(msg);
            }
        }
    }
    else return false;
    return notifier_->Notify(channel_id);
}

} // data
} // rcmw
} // hnu

#endif
