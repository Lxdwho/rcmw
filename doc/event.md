# Event
Event用来对事件进行记录，
## 1 EventBase、TransportEvent
在Evnet中，首先定义了几种事件类型：调度事件、传输事件以及尝试添加事件。并且给出了数据传输时的状态枚举
```cpp
enum class EventType { SCHED_EVENT = 0, TRANS_EVENT = 1, TRY_FETCH_EVENT = 3};
enum class TransPerf {
    TRANSMIT_BEGIN  = 0,
    SERIALIZE       = 1,
    SEND            = 2,
    MESSAGE_ARRIVE  = 3,
    OBTAIN          = 4,
    DESERIALIZE     = 5,
    DISPATCH        = 6,
    NOTIFY          = 7,
    FETCH           = 8,
    CALLBACK        = 9,
    TRANS_END
};
```
在类EventBase中维护了三个变量，并且给出了一系列操作函数以及一个序列化的纯虚函数。
```cpp
int etype_;         // 事件类型
int eid_;           // 事件ID
uint64_t stamp_;    // 
```
`TransportEvent`类继承了`EventBase`，维护了下面三个变量
```cpp
std::string adder_ = "";
uint64_t msg_seq_ = 0;
uint64_t channle_id_ = std::numeric_limits<uint64_t>::max();
```












