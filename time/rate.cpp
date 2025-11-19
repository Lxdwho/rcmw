/**
 * @brief 
 * @date 2025.11.18
 */

#include "rate.h"
#include "rcmw/logger/log.h"

namespace hnu    {
namespace rcmw   {

Rate::Rate(double frequency) : 
    start_(Time::Now()), 
    expected_cycle_time_(1.0 / frequency),
    actual_cycle_time_(0.0) {}

Rate::Rate(uint64_t nanoseconds) : 
    start_(Time::Now()), 
    expected_cycle_time_(static_cast<int64_t>(nanoseconds)),
    actual_cycle_time_(0.0) {}

Rate::Rate(const Duration& rhs) : 
    start_(Time::Now()), 
    expected_cycle_time_(rhs),
    actual_cycle_time_(0.0) {}

void Rate::Sleep() {
    Time expected_end = start_ + expected_cycle_time_;
    Time actual_end = Time::Now();
    if(actual_end < start_) {
        ADEBUG << "Detect backward jumps in time";
        expected_end = actual_end + expected_cycle_time_;
    }
    Duration sleep_time = expected_end - actual_end;
    actual_cycle_time_ = actual_end - start_;
    start_ = expected_end;
    if(sleep_time < Duration(0.0)) {
        ADEBUG << "Detect forward jumps in time";
        if(actual_end > expected_end + expected_cycle_time_)
            start_ = actual_end;
        return ;
    }
    Time::SleepUntil(expected_end);
}

void Rate::Reset() { start_ = Time::Now(); }
Duration Rate::CycleTime() const { return actual_cycle_time_; }

} // rcmw
} // hnu
