#Time
Time文件夹中包含3个类，duration、time、rate
## 1 duration类
`duration`类是一个纳秒操作类，类内部维护了一个`int64_t`的变量`nanoseconds_`，并且提供了对该变量的一系列运算操作：加减乘、大小比较以及赋值等操作，最后提供了一个`sleep`函数用于对调用该函数的线程进行休眠，休眠时间就是变量`nanoseconds_`的值
```cpp
void Duration::Sleep() const {
    auto sleep_time = std::chrono::nanoseconds(nanoseconds_);
    std::this_thread::sleep_for(sleep_time);
}
```

## 2 Time类
`Time`类内部维护了一个`uint64_t`类型的变量`nanoseconds_`，提供了加减以及比较的相关函数，相当于`duration`的上一级封装。
在类外重载了`<<`，其实现如下
```cpp
std::ostream& operator<<(std::ostream& os, const Time& rhs) {
    os << rhs.ToString();
    return os;
}
```
该函数用于将Time所维护的`nanosecond_`转化为时间点，最后转为当地时间点进行字符串输出。

## 3 Rate类
Rate类维护了3个变量
```cpp
Time start_;                    // 实例化时的当前时间
Duration expected_cycle_time_;  // 期望的循环时间
Duration actual_cycle_time_;    // 实际经过的时间
```
同时提供了一个`sleep`函数
```cpp
void Rate::Sleep() {
    Time expected_end = start_ + expected_cycle_time_;  // 当前睡眠的结束时间
    Time actual_end = Time::Now();                      // 当前时间
    if(actual_end < start_) {
        // 如果当前节点晚于开始节点，将当前节点改为开始节点，随后继续
        ADEBUG << "Detect backward jumps in time";
        expected_end = actual_end + expected_cycle_time_;
    }
    Duration sleep_time = expected_end - actual_end;    // 所需要的睡眠时间
    actual_cycle_time_ = actual_end - start_;           // 实际已经睡眠的时间
    start_ = expected_end;                              // 指定下一个睡眠时间点
    if(sleep_time < Duration(0.0)) {
        // 如果睡眠时间小于0
        ADEBUG << "Detect forward jumps in time";
        // 如果当前时间大于，下一个循环的结束时间点，重新赋值start_，否则直接返回
        if(actual_end > expected_end + expected_cycle_time_)
            start_ = actual_end;
        return ;
    }
    Time::SleepUntil(expected_end);
}
```
该函数提供较为准确的睡眠操作。
> 需要注意的是，当两个`sleep`函数执行的中间如果执行了其他函数且耗时超过一个睡眠时间时，睡眠将无法正常执行。
