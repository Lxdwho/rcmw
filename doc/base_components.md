# Base 组件设计分析与性能测试

> 本文档记录 `base/` 目录下核心并发组件的设计分析、线程安全评估和性能测试结论。

---

## 1 原子哈希表（AtomicHashMap）

### 1.1 设计概述

`AtomicHashMap` 是一个 header-only 的固定大小无锁哈希表，基于 CAS 实现线程安全。

```cpp
template <typename K, typename V, std::size_t TableSize = 128>
class AtomicHashMap;
```

- **结构**：定长桶数组，每个桶是一个按 key 排序的单向链表
- **操作**：`Has`、`Get`、`Set`（insert-only，无 Delete）
- **线程安全**：通过 `compare_exchange_strong` 实现无锁插入和更新

### 1.2 线程安全分析

| 场景 | 是否安全 | 说明 |
|------|---------|------|
| 增（新 key）+ 增（同桶） | ✅ | CAS 保护链表插入 |
| 增 + 查 | ✅ | 读者要么看到新节点，要么看不到，不会访问已释放内存 |
| 查 + 查 | ✅ | 无锁遍历，天然安全 |
| 改（同 key Set）+ 查 | ⚠️ | CAS 替换 value_ptr 后 delete 旧值，读者持有的指针可能悬垂 |

**实际使用场景**：项目中 `AtomicHashMap` 主要用于注册表（channel/node/task ID 映射），属于"启动时写入，运行时只读"的模式，不存在"改+查"的竞争，实际安全。

### 1.3 已知问题

- `Get(key, V**)` 返回裸指针，存在悬垂风险（理论问题，实际使用中未触发）
- `std::forward<V>` 误用（第 61、77、192 行），应替换为 `std::move`
- 哈希函数 `key & mode_num_` 过于简单，key 规律分布时容易冲突

---

## 2 原子读写锁（AtomicRWLock）

### 2.1 设计概述

`AtomicRWLock` 基于 `std::atomic<int32_t>` 实现的用户态读写锁。

```cpp
class AtomicRWLock {
    std::atomic<uint32_t> write_lock_wait_num_ = { 0 };
    std::atomic<int32_t> lock_num_ = { 0 };
    bool write_first_ = true;
};
```

- `lock_num_ > 0`：有 N 个读锁
- `lock_num_ == -1`：有写锁
- `write_first_`：写优先策略，写等待时阻塞新读锁

### 2.2 性能测试结论

测试对比了 `std::mutex`、`std::atomic_flag` 自旋锁和 `AtomicRWLock`，在不同线程数、读写比例、临界区长度下的吞吐量。

**短临界区（~10ns）+ 多线程**：
- `std::mutex` 表现最好（futex 睡眠不浪费 CPU）
- `AtomicRWLock` 与 mutex 持平或略差（每次读锁需额外检查 `write_lock_wait_num_`）
- 自旋锁随线程数增加性能下降最快（忙等占用 CPU）

**长临界区（~200ns）+ 读多写少**：
- `AtomicRWLock` 表现最好（读读并发，3.3x 于 mutex）
- 自旋锁次之
- `std::mutex` 最差（所有读写串行）

**结论**：在项目实际场景（注册表读多写少，临界区短）下，`std::mutex` 与 `AtomicRWLock` 性能相当，选择哪个都可以。

---

## 3 有界无锁队列（BoundedQueue）

### 3.1 设计概述

`BoundedQueue` 是基于 LMAX Disruptor 思路的环形缓冲区无锁队列，使用三游标设计。

```cpp
template <typename T>
class BoundedQueue {
    alignas(64) std::atomic<uint64_t> head_ = { 0 };   // 最后已消费位置
    alignas(64) std::atomic<uint64_t> tail_ = { 1 };   // 下一个可写位置
    alignas(64) std::atomic<uint64_t> commit_ = { 1 }; // 最后已提交位置
    T* pool_;           // 预分配对象池
    uint64_t pool_size_;
};
```

**三步入队**：
1. CAS `tail_` 占位
2. 写数据到 `pool_[index]`（无锁，并行）
3. CAS `commit_` 按序提交（串行点）

**设计特点**：
- 预分配对象池，运行时零堆分配
- cache line 对齐，避免伪共享
- Wait 策略解耦（Sleep/Yield/BusySpin/Block）
- 单条 commit 保证消费者看到的数据一定是完整的

### 3.2 线程安全分析

| 操作 | 线程安全 | 说明 |
|------|---------|------|
| Enqueue vs Enqueue | ✅ | CAS tail_ + CAS commit_ 保护 |
| Dequeue vs Dequeue | ✅ | CAS head_ 保护 |
| Enqueue vs Dequeue | ✅ | 操作不同游标，通过 commit_ 同步 |
| `Size()` | ⚠️ | 非原子读取，返回大概值 |
| `Empty()` | ⚠️ | 依赖不精确的 Size() |

### 3.3 BlockWaitStrategy 兼容性问题

`BlockWaitStrategy` 使用 `condition_variable` 阻塞等待，但队列内部没有条件变量。当生产者和消费者同时阻塞时，会出现通知丢失导致的死锁。

**根因**：队列的无锁设计和 `condition_variable` 的阻塞设计语义不匹配。`SleepWaitStrategy`（轮询）与队列兼容。

### 3.4 性能测试结论

对比 `BoundedQueue`（无锁）与 `MutexQueue`（mutex + std::queue）：

**小对象 int（4B）**：
- 核数以内（≤16 线程）：无锁 1.2x ~ 1.7x 优势
- 超核数（24P24C）：无锁崩溃（忙等 CAS 占满 CPU）

**大对象 LargeObject（1KB）**：
- 核数以内：无锁 3x ~ 5x 优势（memcpy 不在锁内，并行拷贝）
- 超核数：无锁同样崩溃

**消息对象 std::string（~500B）**：
- 全场景无锁 2x ~ 4x 优势
- 超核数时**不崩溃**（堆分配的 malloc 内部锁天然退避，降低 commit_ 竞争）

**结论**：通信中间件消息体通常为 string/shared_ptr（涉及堆分配），正好落在无锁队列最优区间。线程数超过核数时应避免使用无锁队列。

---

## 4 无界无锁队列（UnboundedQueue）

### 4.1 设计概述

`UnboundedQueue` 是基于 Michael-Scott 队列思路的链表无锁队列。

```cpp
template <class T>
class UnboundedQueue {
    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;
    std::atomic<size_t> size_;
};
```

- **结构**：单向链表 + 哨兵节点
- **入队**：CAS `tail_` + 链表接续
- **出队**：读 `head_->next` + store `head_`（单消费者）
- **内存管理**：原子引用计数

### 4.2 设计定型：MPSC

队列设计为**多生产者单消费者（MPSC）**：
- `Enqueue`：多线程安全，CAS `tail_` 保护
- `Dequeue`：单线程安全，直接 store `head_`（不需要 CAS）
- 多消费者需要外部 mutex 串行化 `Dequeue`

### 4.3 修复的 bug

**Bug 1：release() double free**

原实现：
```cpp
void release() {
    ref_count.fetch_sub(1);       // ① 原子减
    if(ref_count.load() == 0) {   // ② 再读一次，和 ① 之间有竞争
        delete this;
    }
}
```

修复：
```cpp
void release() {
    if(ref_count.fetch_sub(1) == 1) {  // 返回旧值，一步判断
        delete this;
    }
}
```

**Bug 2：多消费者 Dequeue use-after-free**

原实现使用 CAS 推进 `head_`，但 `load head_` 到 `old_head->next` 之间，节点可能被另一个消费者 CAS 推进后 `release` 删除。修复为单消费者设计，Dequeue 使用 `store` 替代 CAS。

### 4.4 引用计数方案

每个节点 `ref_count` 初始为 2，分别被 `head_`/`tail_` 和前驱节点的 `next` 指向：

| 操作 | ref_count 变化 |
|------|---------------|
| 节点创建 | 2（tail_ + 前驱 next） |
| Enqueue 新节点 | 2→3（tail_ + 前驱 next + 新节点的 next 即将设置） |
| Enqueue 释放旧 tail | 3→2 或 2→1（tail_ 引用转移） |
| Dequeue 释放旧 head | 释放 head_ 引用 |
| 最终释放 | 0 → delete |

### 4.5 性能测试结论

对比 `UnboundedQueue`（无锁 MPSC）与 `MutexQueueMPSC`（mutex + std::queue）：

| 场景 | 结论 |
|------|------|
| 1P1C（无竞争） | mutex 更快（无锁的原子操作开销 > mutex 无竞争时的单次 CAS） |
| 4P1C ~ 16P1C | 无锁队列优势随生产者增加而增大（CAS 竞争 vs mutex 竞争） |

**结论**：无锁队列的优势在多生产者竞争时体现。单生产者场景下 mutex 更简洁高效。

---

## 5 总结：组件选型建议

| 场景 | 推荐方案 |
|------|---------|
| 注册表（启动写，运行读） | `AtomicHashMap`（当前设计满足需求） |
| 临界区短，读多写少 | `std::mutex` 或 `AtomicRWLock`（性能相当） |
| 临界区长，读多写少 | `AtomicRWLock`（读读并发优势明显） |
| 消息队列，线程数 ≤ 核数 | `BoundedQueue`（无锁，并行拷贝） |
| 消息队列，线程数 > 核数 | `std::mutex` + queue（futex 睡眠不浪费 CPU） |
| 多生产者单消费者 | `UnboundedQueue`（MPSC 无锁） |
| 多生产者多消费者 | `BoundedQueue` 或 mutex 队列（UnboundedQueue 不支持多消费者） |
