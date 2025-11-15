/**
 * @brief 共享内存工厂
 * @date 2025.11.14
 */

#ifndef _SEGMENT_FACTORY_H_
#define _SEGMENT_FACTORY_H_

#include "segment.h"

namespace hnu       {
namespace rcmw      {
namespace transport {

class SegmentFactory {
public:
    static SegmentPtr CreateSegment(uint64_t channel_id);
};

} // transport
} // rcmw
} // hnu

#endif
