/**
 * @brief 
 * @date 2025.11.18
 */

#ifndef _TIME_RATE_H_
#define _TIME_RATE_H_

#include "time.h"
#include "duration.h"

namespace hnu    {
namespace rcmw   {

class Rate {
public:
    explicit Rate(double frequency);
    explicit Rate(uint64_t nanoseconds);
    explicit Rate(const Duration& rhs);
    void Sleep();
    void Reset();
    Duration CycleTime() const;
    Duration ExpectedCycleTime() const { return expected_cycle_time_; }
private:
    Time start_;
    Duration expected_cycle_time_;
    Duration actual_cycle_time_;
};

} // rcmw
} // hnu

#endif
