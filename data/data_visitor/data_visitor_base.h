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
#include "common/global_data.h"
#include "logger/log.h"
#include "data/data_notifier.h"

namespace hnu   {
namespace rcmw  {
namespace data  {

class DataVisitorBase {
public:
    DataVisitorBase() : notifier_(new Notifier()) {}

    void RegisterNotifyCallback(std::function<void()>&& callback) {
        notifier_->callback = std::move(callback);
    }
protected:
    DataVisitorBase(const DataVisitorBase& other) = delete;
    DataVisitorBase& operator=(const DataVisitorBase& other) = delete;
    uint64_t next_msg_index_ = 0;
    DataNotifier* data_notifier_ = DataNotifier::Instance();
    std::shared_ptr<Notifier> notifier_;
};

} // data
} // rcmw
} // hnu

#endif
