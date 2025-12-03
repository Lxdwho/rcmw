# Event
`Event`用来对事件进行记录，其中`EventBase`类描述了基本类型，而`TransportEvent`类描述了传输事件，eventcache则负责将事件进行记录。总体而言Event文件夹中的代码主要作用是将cmw运行过程中的传输事件进行记录到txt文本中。
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
在类EventBase中维护了三个变量，并且给出了一系列操作函数以及一个序列化为字符串的纯虚函数。
```cpp
int etype_;         // 事件类型
int eid_;           // 事件ID
uint64_t stamp_;    // 
```
`TransportEvent`类继承了`EventBase`，额外维护了下面三个变量，均是通信事件的相关信息。
```cpp
std::string adder_ = "";
uint64_t msg_seq_ = 0;
uint64_t channle_id_ = std::numeric_limits<uint64_t>::max();
```
并且重写了函数`SerializeTostring`，用于存储事件信息
```cpp
std::string SerializeTostring() override {
        std::stringstream ss;
        ss << etype_ << "\t";
        ss << eid_ << "\t";
        ss << common::GlobalData::GetChannelById(channle_id_) << "\t";
        ss << msg_seq_ << "\t";
        ss << stamp_ << "\t";
        ss << adder_ << "\t";
        return ss.str();
    }
```
## 2 PerfEventCache
`PerfEventCache`类将产生的事件写入到一个文件中`rcmw_perf_.data`。类内维护了两个状态变量`enable_`以及`shutdown_`，分别用于使能事件记录以及事件记录的结束。同时提供了一个字符串变量用于存储事件记录文件的名称`perf_file_`，以及事件记录的最大缓存数量`KFlushSize`，拿到的事件首先是在缓存中，当缓存数量超过`KFlushSize`时一次性写入到文件，最后是维护了一个有界无锁队列`base::BoundedQueue<EventBasePtr> event_queue_;`,并给出了队列的大小`KEventQueueSize`,所有的事件会被存储到这个队列中。
`PerfEventCache`类的业务逻辑大体是，在构造函数中，首先调用了`Start`函数，初始化了事件记录文件名称，随后创建了一个线程对有界对列进行出队操作，将出队的事件记录到文件中。而事件产生者则调用`AddTransportEvent`去向队列中添加事件，以此进行事件的记录。








