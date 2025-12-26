/**
 * @brief 数据访问类基类
 * @date 2025.12.26
 */

#ifndef _RCMW_DATA_DATA_VISITOR_BASE_H_
#define _RCMW_DATA_DATA_VISITOR_BASE_H_

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>
#include "rcmw/common/global_data.h"
#include "rcmw/logger/log.h"
#include "rcmw/data/data_notifier.h"

namespace hnu   {
namespace rcmw  {
namespace data  {

class DataVistorBase {
public:
    DataVistorBase() : notifier_(new Notifier()) {}

    void RegisterNotifyCallback(std::function<void()>&& callback) {
        notifier_->callback = std::move(callback);
    }
protected:
    DataVistorBase(const DataVistorBase& other) = delete;
    DataVistorBase& operator=(const DataVistorBase& other) = delete;
    uint64_t next_msg_index_ = 0;
    DataNotifier* data_notifier_ = DataNotifier::Instance();
    std::shared_ptr<Notifier> notifier_;
};

} // data
} // rcmw
} // hnu

#endif
