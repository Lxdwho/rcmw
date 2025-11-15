# shm
## 1 Block类
block类内部维护了三个变量
* `uint64_t msg_size_`
* `uint64_t msg_info_size_`
* `std::atomic<int32_t> lock_num_ = { 0 };`
并提供了一系列函数用于实现内存的读写锁
```cpp
bool TryLockForWrite();
bool TryLockForRead();
void ReleaseWriteLock();
void ReleaseReadLock();
```
但是为什么要将`msg_size_`以及`msg_info_size_`放在block中呢？

## 2 State类
State类是共享内存数据块的头部，用于标识这一块内存的使用情况，内部维护的变量如下：
* `std::atomic<bool> need_remap_ = {false};`用于判断当前内存是否需要进行重映射
* `std::atomic<uint32_t> seq_ = { 0 };`代表的是当前正在写的block的索引
* `std::atomic<uint32_t> reference_count_ = { 0 };`内存块的引用计数
* `std::atomic<uint32_t> ceiling_msg_size_;`内存块的msg大小
同时类内部提供了对这些原子变量操作的外部接口
```cpp
    void DecreaseReferenceCount() {
        uint32_t current_reference_count = reference_count_.load();
        do{
            if(current_reference_count == 0) return;
        }
        while(!reference_count_.compare_exchange_strong(
            current_reference_count, current_reference_count - 1));
    }
    /* 增加引用计数 */
    void IncreaseReferenceCount() { reference_count_.fetch_add(1); }
    
    /* 变量操作 */
    uint32_t FetchAddSeq(uint32_t diff) { return seq_.fetch_add(diff); }
    uint32_t seq() { return seq_.load(); }
    
    void set_need_remap(bool need_remap) { need_remap_.store(need_remap); }
    bool need_remap() { return need_remap_.load(); }
    
    uint64_t ceiling_msg_size() { return ceiling_msg_size_.load(); }
    uint32_t reference_count() { return reference_count_.load(); }
```
## 3 ShmConf类
`ShmConf`类的主要作用是根据msg的大小为所需要申请的内存块计算大小
其内部维护了四个变量
* `uint64_t ceiling_msg_size_;`消息大小上限
* `uint64_t block_buf_size_;`每个block的buf大小
* `uint64_t block_num_;`
* `uint64_t managed_shm_size_;`管理的共享内存大小
并通过`Update`函数对他们进行计算，其构造函数以及`Update`函数如下
```cpp
ShmConf::ShmConf() { Update(MESSAGE_SIZE_16K); }
ShmConf::ShmConf(const uint64_t& real_msg_size) { Update(MESSAGE_SIZE_16K); }

void ShmConf::Update(const uint64_t& real_msg_size) {
    ceiling_msg_size_ = GetCeilingMessageSize(real_msg_size);ceiling_msg_size_
    block_buf_size_ = GetBlockBufSize(ceiling_msg_size_);           
    block_num_ = GetBlockNum(ceiling_msg_size_);
    managed_shm_size_ = EXTRA_SIZE + STATE_SIZE + (BLOCK_SIZE + block_buf_size_) * block_num_;
}
```
该类在构建时，需要一个`msg_size`去确定内部维护的四个变量的大小，在`Update`内部去确定所维护变量的大小，大小的具体计算如下：
```cpp
uint64_t ShmConf::GetCeilingMessageSize(const uint64_t& real_msg_size) {
    uint64_t ceiling_msg_size = MESSAGE_SIZE_16K;
    if(real_msg_size <= MESSAGE_SIZE_16K) {
        ceiling_msg_size = MESSAGE_SIZE_16K;
    }
    else if(real_msg_size <= MESSAGE_SIZE_128K) {
        ceiling_msg_size = MESSAGE_SIZE_128K;
    }
    else if (real_msg_size <= MESSAGE_SIZE_1M) {
        ceiling_msg_size = MESSAGE_SIZE_1M;
    }
    else if(real_msg_size <= MESSAGE_SIZE_8M) {
        ceiling_msg_size = MESSAGE_SIZE_8M;
    }
    else if(real_msg_size <= MESSAGE_SIZE_16M) {
        ceiling_msg_size = MESSAGE_SIZE_16M;
    }
    else ceiling_msg_size = MESSAGE_SIZE_MORE;
    return ceiling_msg_size;
}

uint64_t ShmConf::GetBlockBufSize(const uint64_t& ceiling_msg_size) {
    return ceiling_msg_size + MESSAGE_INFO_SIZE;
}

uint64_t ShmConf::GetBlockNum(const uint64_t& ceiling_msg_size) {
    uint32_t num = 0;
    switch(ceiling_msg_size) {
    case MESSAGE_SIZE_16K:
        num = BLOCK_NUM_16K;
        break;
    case MESSAGE_SIZE_128K:
        num = BLOCK_NUM_128K;
        break;
    case MESSAGE_SIZE_1M:
        num = BLOCK_NUM_1M;
        break;
    case MESSAGE_SIZE_8M:
        num = BLOCK_NUM_8M;
        break;
    case MESSAGE_SIZE_16M:
        num = BLOCK_NUM_16M;
        break;
    case MESSAGE_SIZE_MORE:
        num = BLOCK_NUM_MORE;
        break;
    default:
        AWARN << "unknow ceilint_msg_size[" << ceiling_msg_size << "]";
        break;
    }
    return num;
}

// Extra size, Byte
const uint64_t ShmConf::EXTRA_SIZE          = 1024 * 4;
// State size, Byte
const uint64_t ShmConf::STATE_SIZE          = 1024;
// Block size, Byte
const uint64_t ShmConf::BLOCK_SIZE          = 1024;
// Message info size, Byte
const uint64_t ShmConf::MESSAGE_INFO_SIZE   = 1024;
// For message 0-10K
const uint32_t ShmConf::BLOCK_NUM_16K       = 512;
const uint64_t ShmConf::MESSAGE_SIZE_16K    = 1024 * 16;
// For message 10-100K
const uint32_t ShmConf::BLOCK_NUM_128K      = 128;
const uint64_t ShmConf::MESSAGE_SIZE_128K   = 1024 * 128;
// For message 100-1M
const uint32_t ShmConf::BLOCK_NUM_1M        = 64;
const uint64_t ShmConf::MESSAGE_SIZE_1M     = 1024 * 1024;
// For message 1-6M
const uint32_t ShmConf::BLOCK_NUM_8M        = 32;
const uint64_t ShmConf::MESSAGE_SIZE_8M     = 1024 * 1024 * 8;
// For message 6-10M
const uint32_t ShmConf::BLOCK_NUM_16M       = 16;
const uint64_t ShmConf::MESSAGE_SIZE_16M    = 1024 * 1024 * 16;
// For message 10M+
const uint32_t ShmConf::BLOCK_NUM_MORE      = 8;
const uint64_t ShmConf::MESSAGE_SIZE_MORE   = 1024 * 1024 * 32;
```
`Update`类通过调用上述几个函数计算得到最后要开辟的共享内存的大小`managed_shm_size_`
## 4 readable_info类
`readable_info`类如下
```cpp
class ReadableInfo{
public:
    ReadableInfo();
    ReadableInfo(uint64_t host_id, uint32_t block_index, uint64_t channel_id);
    virtual ~ReadableInfo();

    ReadableInfo& operator=(ReadableInfo& other);

    bool DeserializeFrom(const std::string& src);
    bool DeserializeFrom(const char* src, std::size_t len);
    bool SerializeTo(std::string* dst) const;

    uint64_t host_id() const { return host_id_; }
    void set_host_id(uint64_t host_id) { host_id_ = host_id; }

    uint32_t block_index() const { return block_index_; }
    void set_block_index(uint32_t block_index) { block_index_ = block_index; }

    uint64_t channel_id() const { return channel_id_; }
    void set_channel_id(uint64_t channel_id) { channel_id_ = channel_id; }

    static const size_t ksize;

private:
    uint64_t host_id_;
    uint32_t block_index_;
    uint64_t channel_id_;
};
```
类内主要维护了主机、话题id以及一个`block_index_`，并提供了对他们的操作函数以及序列化与反序列化函数，具体的作用未知。
## 5 Segment
CMW中提供了两种共享内存的实现方式，他们都继承了一个基类`Segment`.
```cpp
/* 内存可写块结构体 */
struct WritableBlock
{
    uint32_t index = 0;
    Block* block = nullptr;
    uint8_t* buf = nullptr;
};
using ReadableBlock = WritableBlock;

class Segment {
public:
    explicit Segment(uint64_t channel_id);
    virtual ~Segment() {}

    bool AcquireBlockToWrite(std::size_t msg_size, WritableBlock* writable_block);
    void ReleaseWrittenBlock(const WritableBlock& writable_block);

    bool AcquireBlockToRead(ReadableBlock* readable_block);
    void ReleaseReadableBlock(const ReadableBlock* readable_block);

protected:
    virtual bool Destory();
    virtual void Reset() = 0;
    virtual bool Remove() = 0;
    virtual bool OpenOnly() = 0;
    virtual bool OpenOrCreate() = 0;
    
    bool init_;
    ShmConf conf_;
    uint64_t channel_id_;

    State* state_;
    Block* blocks_;
    void* managed_shm_;
    std::mutex block_buf_lock_;
    std::unordered_map<uint32_t, uint8_t*> block_buf_addrs_;

private:
    bool Remap();
    bool Recreate(const uint64_t& msg_size);
    uint32_t GetNextWritableBlockIndex();
};
```
`Segment.h`文件中首先定义了一个结构体`WritableBlock`，该结构体用于存储要进行读写的块。随后在类内，实现了几个个函数用于对块进行获取、管理。
```cpp
    bool AcquireBlockToWrite(std::size_t msg_size, WritableBlock* writable_block);
    void ReleaseWrittenBlock(const WritableBlock& writable_block);

    bool AcquireBlockToRead(ReadableBlock* readable_block);
    void ReleaseReadableBlock(const ReadableBlock* readable_block);
    
    virtual bool Destory();
    bool Remap();
    bool Recreate(const uint64_t& msg_size);
    uint32_t GetNextWritableBlockIndex();
```


















