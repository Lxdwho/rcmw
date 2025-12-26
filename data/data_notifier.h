/**
 * @brief 数据通知器
 * @date 2025.12.26
 */

#ifndef _RCMW_DATA_DATA_NOTIFIER_H_
#define _RCMW_DATA_DATA_NOTIFIER_H_

#include "rcmw/logger/log.h"
#include "rcmw/common/macros.h"
#include "rcmw/data/buffer/cache_buffer.h"
#include "rcmw/time/time.h"
#include "rcmw/base/atomic_hash_map.h"
#include <mutex>
#include <memory>
#include <vector>

namespace hnu   {
namespace rcmw  {
namespace data  {

using hnu::rcmw::Time;
using hnu::rcmw::base::AtomicHashMap;

/**
 * @brief 通知器：内部仅封装了一个void()的function
 */
struct Notifier {
    std::function<void()> callback;
};

/**
 * @brief 数据通知器：
 */
class DataNotifier {
public:
    using NotifyVector = std::vector<std::shared_ptr<Notifier>>;
    ~DataNotifier() {}

    void AddNotifier(uint64_t channel_id, const std::shared_ptr<Notifier>& notifier);
    bool Notify(const uint64_t channel_id);
private:
    std::mutex notifier_map_mutex_;
    AtomicHashMap<uint64_t, NotifyVector> notifiers_map_;
    DECLARE_SINGLETON(DataNotifier);
};

inline DataNotifier::DataNotifier() {}

inline void DataNotifier::AddNotifier(uint64_t channel_id, const std::shared_ptr<Notifier>& notifier) {
    std::lock_guard<std::mutex> lock(notifier_map_mutex_);
    NotifyVector* notifiers = nullptr;
    if(notifiers_map_.Get(channel_id, &notifiers)) {
        notifiers->emplace_back(notifier);
    }
    else {
        NotifyVector new_notifier = {notifier};
        notifiers_map_.Set(channel_id, new_notifier);
    }
}

inline bool DataNotifier::Notify(const uint64_t channel_id) {
    NotifyVector* notifies = nullptr;
    if(notifiers_map_.Get(channel_id, &notifies)) {
        for(auto& notifier : *notifies) {
            if(notifier && notifier->callback) notifier->callback();
        }
        return true;
    }
    return false;
}

} // data
} // rcmw
} // hnu

#endif
