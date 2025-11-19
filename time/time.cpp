/**
 * @brief 时间操作类，维护了一个纳秒变量
 * @date 2025.11.18
 */

#include "time.h"
#include <iostream>
#include <limits>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <thread>

namespace hnu    {
namespace rcmw   {

using std::chrono::high_resolution_clock;
using std::chrono::steady_clock;
using std::chrono::system_clock;

const Time Time::MAX = Time(std::numeric_limits<uint64_t>::max());
const Time Time::MIN = Time(0);

Time::Time(uint64_t nanoseconds) { nanoseconds_ = nanoseconds; }

Time::Time(int nanoseconds) {
    nanoseconds_ = static_cast<uint64_t>(nanoseconds);
}

Time::Time(double seconds) {
    nanoseconds_ = static_cast<uint64_t>(seconds * 1000000000UL);
}

Time::Time(u_int32_t seconds, u_int32_t nanoseconds) {
    nanoseconds_ = static_cast<uint64_t>(seconds * 1000000000UL)
                    + nanoseconds;
}

Time::Time(const Time& other) {
    nanoseconds_ = other.nanoseconds_;    
}

Time& Time::operator=(const Time& other) {
    nanoseconds_ = other.nanoseconds_;
    return *this;
}

/* 获取当前时间的纳秒，使用high_resolution_clock */
Time Time::Now() {
    auto now = high_resolution_clock::now();
    auto nano_time_point = std::chrono::time_point_cast
            <std::chrono::nanoseconds>(now);
    auto epoch = nano_time_point.time_since_epoch();
    uint64_t now_nano = std::chrono::duration_cast<
            std::chrono::nanoseconds>(epoch).count();
    return Time(now_nano);
}

/* 获取当前时间的纳秒，使用steady_clock */
Time Time::MonoTime() {
    auto now = steady_clock::now();
    auto nano_time_point = std::chrono::time_point_cast<
            std::chrono::nanoseconds>(now);
    auto epoch = nano_time_point.time_since_epoch();
    uint64_t now_nano = std::chrono::duration_cast<
            std::chrono::nanoseconds>(epoch).count();
    return Time(now_nano);
}

/* 睡眠当前线程到指定的时间点 */
void Time::SleepUntil(const Time& time) {
    auto nano = std::chrono::nanoseconds(time.ToNanosecond());
    system_clock::time_point tp(nano);
    std::this_thread::sleep_until(tp);
}

double Time::ToSecond() const {
    return static_cast<double>(nanoseconds_) / 1000000000UL;
}

uint64_t Time::Tomicrosecond() const {
    return static_cast<uint64_t>(nanoseconds_ / 1000.0);
}

uint64_t Time::ToNanosecond() const {
    return nanoseconds_;
}

std::string Time::ToString() const {
    auto nano = std::chrono::nanoseconds(nanoseconds_);
    system_clock::time_point tp(nano);
    auto time = system_clock::to_time_t(tp);
    struct tm stm;
    auto ret = localtime_r(&time, &stm);
    if(ret == nullptr) return std::to_string(
        static_cast<double>(nanoseconds_) / 1000000000.0);
    std::stringstream ss;
#if __GNUC__ >= 5
    ss << std::put_time(ret, "%F %T");
    ss << "." << std::setw(9) << std::setfill('0') 
        << nanoseconds_ % 1000000000UL;
#else
    /* 旧版本 */
    char date_time[128];
    strftime(date_time, sizeof(date_time), "%F %T", ret);
    ss << std::string(date_time) << '.' << std::setw(9) 
        << std::setfill('0') << nanoseconds_ % 1000000000UL;
#endif
    return ss.str();
}

bool Time::IsZero() const { return nanoseconds_ == 0; }

Duration Time::operator-(const Time &rhs) const {
    return Duration(static_cast<int64_t>
            (nanoseconds_ - rhs.nanoseconds_));
}

Time Time::operator+(const Duration &rhs) const {
    return Time(nanoseconds_ + rhs.ToNanosecond());
}

Time Time::operator-(const Duration &rhs) const {
    return Time(nanoseconds_ - rhs.ToNanosecond());
}

Time Time::operator+=(const Duration &rhs) {
    *this = *this + rhs;
    return *this;
}

Time Time::operator-=(const Duration &rhs) {
    *this = *this - rhs;
    return *this;
}

bool Time::operator==(const Time& rhs) const {
    return nanoseconds_ == rhs.nanoseconds_;
}

bool Time::operator!=(const Time& rhs) const {
    return nanoseconds_ != rhs.nanoseconds_;
}

bool Time::operator>(const Time& rhs) const {
    return nanoseconds_ > rhs.nanoseconds_;
}

bool Time::operator<(const Time& rhs) const {
    return nanoseconds_ < rhs.nanoseconds_;
}

bool Time::operator>=(const Time& rhs) const {
    return nanoseconds_ >= rhs.nanoseconds_;
}

bool Time::operator<=(const Time& rhs) const {
    return nanoseconds_ <= rhs.nanoseconds_;
}

std::ostream& operator<<(std::ostream& os, const Time& rhs) {
    os << rhs.ToString();
    return os;
}

} // rcmw
} // hnu
