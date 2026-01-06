/**
 * @brief 数据访问类
 * @date 2025.12.26
 */

#ifndef _RCMW_DATA_DATA_VISITOR_H_
#define _RCMW_DATA_DATA_VISITOR_H_

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>
#include "rcmw/data/data_visitor/data_visitor_base.h"
#include "rcmw/data/data_fusion/all_latest.h"
#include "rcmw/data/buffer/channel_buffer.h"
#include "rcmw/data/data_dispatcher.h"
#include "rcmw/common/types.h"
#include "rcmw/logger/log.h"

namespace hnu   {
namespace rcmw  {
namespace data  {

struct VisitorConfig {
    VisitorConfig(uint64_t id, uint32_t size) : channel_id(id), queue_size(size) {}
    uint64_t channel_id;
    uint32_t queue_size;
};

template<typename T>
using BufferType = CacheBuffer<std::shared_ptr<T>>;

template<typename M0, typename M1 = NullType, typename M2 = NullType, typename M3 = NullType>
class DataVisitor : public DataVisitorBase {
public:
    explicit DataVisitor(const std::vector<VisitorConfig>& config) :
                buffer_m0_(config[0].channel_id, new BufferType<M0>(config[0].queue_size)),
                buffer_m1_(config[1].channel_id, new BufferType<M1>(config[1].queue_size)),
                buffer_m2_(config[2].channel_id, new BufferType<M2>(config[2].queue_size)),
                buffer_m3_(config[3].channel_id, new BufferType<M3>(config[3].queue_size)) {

        DataDispatcher<M0>::Instance()->AddBuffer(buffer_m0_);
        DataDispatcher<M1>::Instance()->AddBuffer(buffer_m1_);
        DataDispatcher<M2>::Instance()->AddBuffer(buffer_m2_);
        DataDispatcher<M3>::Instance()->AddBuffer(buffer_m3_);
        
        data_notifier_->AddNotifier(buffer_m0_.channel_id(), notifier_);

        data_fusion_ = new fusion::AllLatest<M0, M1, M2, M3>(buffer_m0_, buffer_m1_, buffer_m2_, buffer_m3_);
    }

    ~DataVisitor() {
        if(data_fusion_) {
            delete data_fusion_;
            data_fusion_ = nullptr;
        }
    }

    bool TryFetch(std::shared_ptr<M0>& m0, std::shared_ptr<M1>& m1, 
                  std::shared_ptr<M2>& m2, std::shared_ptr<M3>& m3) {
        if(data_fusion_->Fusion(&next_msg_index_, m0, m1, m2, m3)) {
            next_msg_index_++;
            return true;
        }
        return false;
    }
private:
    fusion::DataFusion<M0, M1, M2, M3>* data_fusion_ = nullptr;
    ChannelBuffer<M0> buffer_m0_;
    ChannelBuffer<M1> buffer_m1_;
    ChannelBuffer<M2> buffer_m2_;
    ChannelBuffer<M3> buffer_m3_;
};

template<typename M0, typename M1, typename M2>
class DataVisitor<M0, M1, M2, NullType> : public DataVisitorBase {
public:
    explicit DataVisitor(const std::vector<VisitorConfig>& config) :
                buffer_m0_(config[0].channel_id, new BufferType<M0>(config[0].queue_size)),
                buffer_m1_(config[1].channel_id, new BufferType<M1>(config[1].queue_size)),
                buffer_m2_(config[2].channel_id, new BufferType<M2>(config[2].queue_size)) {

        DataDispatcher<M0>::Instance()->AddBuffer(buffer_m0_);
        DataDispatcher<M1>::Instance()->AddBuffer(buffer_m1_);
        DataDispatcher<M2>::Instance()->AddBuffer(buffer_m2_);
        
        data_notifier_->AddNotifier(buffer_m0_.channel_id(), notifier_);

        data_fusion_ = new fusion::AllLatest<M0, M1, M2>(buffer_m0_, buffer_m1_, buffer_m2_);
    }

    ~DataVisitor() {
        if(data_fusion_) {
            delete data_fusion_;
            data_fusion_ = nullptr;
        }
    }

    bool TryFetch(std::shared_ptr<M0>& m0, std::shared_ptr<M1>& m1, 
                  std::shared_ptr<M2>& m2) {
        if(data_fusion_->Fusion(&next_msg_index_, m0, m1, m2)) {
            next_msg_index_++;
            return true;
        }
        return false;
    }
private:
    fusion::DataFusion<M0, M1, M2>* data_fusion_ = nullptr;
    ChannelBuffer<M0> buffer_m0_;
    ChannelBuffer<M1> buffer_m1_;
    ChannelBuffer<M2> buffer_m2_;
};


template<typename M0, typename M1>
class DataVisitor<M0, M1, NullType, NullType> : public DataVisitorBase {
public:
    explicit DataVisitor(const std::vector<VisitorConfig>& config) :
                buffer_m0_(config[0].channel_id, new BufferType<M0>(config[0].queue_size)),
                buffer_m1_(config[1].channel_id, new BufferType<M1>(config[1].queue_size)) {

        DataDispatcher<M0>::Instance()->AddBuffer(buffer_m0_);
        DataDispatcher<M1>::Instance()->AddBuffer(buffer_m1_);
        
        data_notifier_->AddNotifier(buffer_m0_.channel_id(), notifier_);

        data_fusion_ = new fusion::AllLatest<M0, M1>(buffer_m0_, buffer_m1_);
    }

    ~DataVisitor() {
        if(data_fusion_) {
            delete data_fusion_;
            data_fusion_ = nullptr;
        }
    }

    bool TryFetch(std::shared_ptr<M0>& m0, std::shared_ptr<M1>& m1) {
        if(data_fusion_->Fusion(&next_msg_index_, m0, m1)) {
            next_msg_index_++;
            return true;
        }
        return false;
    }
private:
    fusion::DataFusion<M0, M1>* data_fusion_ = nullptr;
    ChannelBuffer<M0> buffer_m0_;
    ChannelBuffer<M1> buffer_m1_;
};


template<typename M0>
class DataVisitor<M0, NullType, NullType, NullType> : public DataVisitorBase {
public:
    explicit DataVisitor(const std::vector<VisitorConfig>& config) :
                buffer_(config[0].channel_id, new BufferType<M0>(config[0].queue_size)) {
        DataDispatcher<M0>::Instance()->AddBuffer(buffer_);
        data_notifier_->AddNotifier(buffer_.channel_id(), notifier_);
    }

    DataVisitor(uint64_t channel_id, uint32_t queue_size):
                buffer_(channel_id, new BufferType<M0>(queue_size)) {
        DataDispatcher<M0>::Instance()->AddBuffer(buffer_);
        data_notifier_->AddNotifier(buffer_.channel_id(), notifier_);
    }

    bool TryFetch(std::shared_ptr<M0>& m0) {
        if(buffer_.Fetch(&next_msg_index_, m0)) {
            next_msg_index_++;
            return true;
        }
        return false;
    }
private:
    ChannelBuffer<M0> buffer_;
};

} // data
} // rcmw
} // hnu

#endif
