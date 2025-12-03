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
`ShmTransmitter`类中，重写了父类的`Enable`函数以及`Disable`函数，其中`Enable`函数被调用时会创建一块共享内存（`Segment`）以及`Nortifier`，并将状态设置为`true`。Disable被调用时则会去将`segment_`以及`notifier_`赋值为空，并设置状态为`false`。
同时类内提供了`Transmit`函数的实现
```cpp
template <typename M>
bool ShmTransmitter<M>::Transmit(const M& msg, const MessageInfo& msg_info) {
    if(!this->enabled_) {
        ADEBUG << "not enable."
        return false;
    }
    WritableBlock wb;
    // ADEBUG << "Debug Serialize start: " << Time::Now().ToMicrosecond();
    serialize::DataStream ds;
    ds << msg;
    std::size_t msg_size = ds.ByteSize();
    // ADEBUG << "Debug Serialize end: " << Time::Now().ToMicrosecond();
    
    if(!segment_->AcquireBlockToWrite(msg_size, &wb)) {
        AERROR << "acquire block failed."
        return false;
    }

    std::memcpy(wb.buf, ds.data(), msg_size);

    wb.block->set_msg_size(msg_size);
    char* msg_info_addr = reinterpret_cast<char*>(wb.buf) + msg_size;

    // 拷贝sender_id
    std::memcpy(msg_info_addr, msg_info.sender_id().data(), ID_SIZE);
    // 拷贝spare_id
    std::memcpy(msg_info_addr + ID_SIZE, msg_info.spaer_id().data(), ID_SIZE);
    // 拷贝seq
    *reinterpret_cast<uint64_t*>(msg_info_addr + ID_SIZE*2) = msg_info.seq_num();

    wb.block->set_msg_info_size(ID_SIZE*2 + sizeof(uint64_t));

    segment_->ReleaseWrittenBlock(wb);

    ReadableInfo readable_info(host_id_, wb.index, channel_id_);
    ADEBUG << "Writing shareedmem message: "
            << common::GlobalData::GetChannelById(channel_id_)
            << "to block: " << wb.index;
    return notifier_->Notify(readable_info);
}
```
`Transmit`函数中首先会去判断类本身的状态`enable_`是否为`true`，是则去从`segment`中获取一块可以写的内存，随后依次将要发送的消息、消息的附带信息写入内存块，最后进行写释放。完成消息的写入后创建一个`readable_info`通过`notifire`发送出去。

## 4 RTPSTransmitter


