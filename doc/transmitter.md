# Transmitter
`Transmitter`用于创建发送者，其中提供了一个基类以及两个派生类，分别用于创建基于SHM以及RTPS的`Transmitter`。而`Transmitter`类也是`Endpoint`的派生类
## 1 Endpoint
在`Endpoint`类中维护了3个变量
```cpp
bool enabled_;          // 端点使能信号
Identity id_;           // 端点标识符
RoleAttributes attr_;   // 角色信息
```
端点标识符中包含一个ID，角色信息中则包含进程ID、话题名、ID等通信基础信息。

## 2 Transmitter基类
`Transmitter`类继承了`Endpoint`类，类内维护了两个变量
```cpp
uint64_t seq_num_;      // 数据帧号
MessageInfo msg_info_;  // 数据自身信息
```
同时类内提供了一些函数
```cpp
/* Transmitter使能与失能 */
virtual void Enable() = 0;
virtual void Disable() = 0;

virtual void Enable(const RoleAttributes& attr);
virtual void Disable(const RoleAttributes& attr);

/* Transmitter发送一帧数据 */
virtual bool Transmit(const MessagePtr& msg);
virtual bool Transmit(const MessagePtr& msg, const MessageInfo& msg_info) = 0;

/* 帧号迭代与查询 */
uint64_t NextSeqNum() { return ++seq_num_; }
uint64_t seq_num() const { return seq_num_; }
```

## 3 ShmTransmitter基类
ShmTransmitter类用于创建基于共享内存的发送端，类内维护了四个变量
```cpp
SegmentPtr segment_;    // 准备写的共享内存
uint64_t channel_id_;   // 发送的通道、话题
uint64_t host_id_;      // 发送主机ID
NotifierPtr notifier_;  // 发送notifier
```



