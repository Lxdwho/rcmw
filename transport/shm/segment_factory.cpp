/**
 * @brief 共享内存工厂
 * @date 2025.11.14
 */

#include "segment_factory.h"
#include "rcmw/common/global_data.h"
#include "posix_segment.h"
#include "xsi_segment.h"
#include "rcmw/logger/log.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

using hnu::rcmw::common::GlobalData;

auto SegmentFactory::CreateSegment(uint64_t channel_id) ->SegmentPtr {
    std::string segment_type(XsiSegment::Type());
    ADEBUG << "segment type: " << segment_type;
    if(segment_type == PosixSegment::Type()) {
        return std::make_shared<PosixSegment>(channel_id);
    }
    return std::make_shared<XsiSegment>(channel_id);
}

} // transport
} // rcmw
} // hnu
