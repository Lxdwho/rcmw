/**
 * @brief 时间操作类，维护了一个纳秒变量
 * @date 2025.11.18
 */

#ifndef _TIME_TIME_H_
#define _TIME_TIME_H_

#include "duration.h"
#include <iostream>

namespace hnu    {
namespace rcmw   {

class Time {
public:
    static const Time MAX;
    static const Time MIN;
    Time() = default;
    explicit Time(uint64_t nanoseconds);
    explicit Time(int nanoseconds);
    explicit Time(double seconds);
    Time(u_int32_t seconds, u_int32_t nanoseconds);
    Time(const Time& other);
    Time& operator=(const Time& other);

    static Time Now();
    static Time MonoTime();
    static void SleepUntil(const Time& time);
    double ToSecond() const;
    uint64_t Tomicrosecond() const;
    uint64_t ToNanosecond() const;
    std::string ToString() const;
    bool IsZero() const;

    Duration operator-(const Time &rhs) const;
    Time operator+(const Duration &rhs) const;
    Time operator-(const Duration &rhs) const;
    Time operator+=(const Duration &rhs);
    Time operator-=(const Duration &rhs);
    bool operator==(const Time& rhs) const;
    bool operator!=(const Time& rhs) const;
    bool operator>(const Time& rhs) const;
    bool operator<(const Time& rhs) const;
    bool operator>=(const Time& rhs) const;
    bool operator<=(const Time& rhs) const;
private:
    uint64_t nanoseconds_ = 0;
};

std::ostream& operator<<(std::ostream& os, const Time& rhs);

} // rcmw
} // hnu

#endif
