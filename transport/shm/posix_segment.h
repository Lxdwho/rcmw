/**
 * @brief 基于posix的共享内存
 * @date 2025.11.14
 */

#include "segment.h"
#include <string>

namespace hnu       {
namespace rcmw      {
namespace transport {

class PosixSegment : public Segment {
public:
    explicit PosixSegment(uint64_t channel_name);
    virtual ~PosixSegment();

    static const char* Type() { return "posix"; }

private:
    void Reset() override;
    bool Remove() override;
    bool OpenOnly() override;
    bool OpenOrCreate() override;
    std::string shm_name_;
};

} // transport
} // rcmw
} // hnu
