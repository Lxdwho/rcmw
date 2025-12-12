/**
 * @brief 基于System v 的segment子类
 * @date 2025.11.13
 */

#ifndef _XSI_SEGMENT_H_
#define _XSI_SEGMENT_H_

#include "segment.h"
#include <sys/types.h>

namespace hnu       {
namespace rcmw      {
namespace transport {

class XsiSegment : public Segment {
public:
    explicit XsiSegment(uint64_t channel_id);
    virtual ~XsiSegment();

    static const char* Type() { return "xsi"; }
private:
    void Reset() override;
    bool Remove() override;
    bool OpenOnly() override;
    bool OpenOrCreate() override;
    key_t key_;
};

} // transport
} // rcmw
} // hnu

#endif
