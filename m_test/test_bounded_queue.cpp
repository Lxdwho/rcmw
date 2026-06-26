/**
 * @brief  BoundedQueue 全面测试
 *
 * 测试项：
 *   1. 基本功能：Init / Enqueue / Dequeue / Size / Empty
 *   2. 队列满：Enqueue 返回 false
 *   3. 队列空：Dequeue 返回 false
 *   4. 单生产者单消费者（SPSC）数据完整性
 *   5. 多生产者单消费者（MPSC）数据完整性
 *   6. 单生产者多消费者（SPMC）数据完整性
 *   7. 多生产者多消费者（MPMC）数据完整性
 *   8. WaitEnqueue / WaitDequeue 阻塞与唤醒
 *   9. BreakAllWait 中断等待
 *  10. 高并发压力测试
 *
 * 编译：g++ -std=c++14 -pthread -O2 -I . m_test/test_bounded_queue.cpp -o test_bounded_queue
 * 运行：./test_bounded_queue
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <atomic>
#include <set>
#include <mutex>
#include <queue>
#include <cstring>
#include <cassert>
#include <chrono>
#include <string>
#include <algorithm>

#include "base/bounded_queue.h"
#include "base/wait_srategy.h"

using hnu::rcmw::base::BoundedQueue;
using hnu::rcmw::base::BlockWaitStrategy;
using hnu::rcmw::base::SleepWaitStrategy;

// ──────────────────────────────────────────────────────────────
// 辅助
// ──────────────────────────────────────────────────────────────
static int g_pass = 0;
static int g_fail = 0;

#define TEST_ASSERT(cond, msg)                                          \
    do {                                                                \
        if (!(cond)) {                                                  \
            std::cerr << "  FAIL: " << msg << "  (" << #cond           \
                      << ") at " << __FILE__ << ":" << __LINE__ << "\n";\
            ++g_fail;                                                   \
        } else { ++g_pass; }                                            \
    } while(0)

#define TEST_SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl;

// ──────────────────────────────────────────────────────────────
// 1. 基本功能
// ──────────────────────────────────────────────────────────────
void TestBasic() {
    TEST_SECTION("1. 基本功能");

    BoundedQueue<int> q;
    TEST_ASSERT(q.Init(8), "Init 成功");

    // 初始状态
    TEST_ASSERT(q.Empty(), "初始为空");
    TEST_ASSERT(q.Size() == 0, "初始 Size=0");

    // 入队出队
    TEST_ASSERT(q.Enqueue(42), "Enqueue 成功");
    TEST_ASSERT(!q.Empty(), "非空");
    TEST_ASSERT(q.Size() == 1, "Size=1");

    int val = 0;
    TEST_ASSERT(q.Dequeue(&val), "Dequeue 成功");
    TEST_ASSERT(val == 42, "值正确");
    TEST_ASSERT(q.Empty(), "取出后为空");

    // 批量入队出队（验证顺序性：单消费者下 FIFO）
    for (int i = 0; i < 8; ++i) {
        TEST_ASSERT(q.Enqueue(i), ("批量入队 " + std::to_string(i)).c_str());
    }
    for (int i = 0; i < 8; ++i) {
        q.Dequeue(&val);
        TEST_ASSERT(val == i, ("FIFO 顺序: 期望 " + std::to_string(i) + " 得到 " + std::to_string(val)).c_str());
    }
}

// ──────────────────────────────────────────────────────────────
// 2. 队列满
// ──────────────────────────────────────────────────────────────
void TestFull() {
    TEST_SECTION("2. 队列满");

    BoundedQueue<int> q;
    q.Init(4);  // 实际 pool = 6，有效容量 4

    for (int i = 0; i < 4; ++i) {
        TEST_ASSERT(q.Enqueue(i), ("入队 " + std::to_string(i)).c_str());
    }
    TEST_ASSERT(!q.Enqueue(99), "队列满时 Enqueue 返回 false");
    TEST_ASSERT(q.Size() == 4, "满后 Size=4");

    // 取出一个后可以再入
    int val;
    q.Dequeue(&val);
    TEST_ASSERT(q.Enqueue(100), "取出一个后再入队成功");
}

// ──────────────────────────────────────────────────────────────
// 3. 队列空
// ──────────────────────────────────────────────────────────────
void TestEmpty() {
    TEST_SECTION("3. 队列空");

    BoundedQueue<int> q;
    q.Init(4);

    int val = -1;
    TEST_ASSERT(!q.Dequeue(&val), "空队列 Dequeue 返回 false");
    TEST_ASSERT(val == -1, "val 未被修改");

    // 入一个再全取完
    q.Enqueue(1);
    q.Dequeue(&val);
    TEST_ASSERT(!q.Dequeue(&val), "取完后再次 Dequeue 返回 false");
}

// ──────────────────────────────────────────────────────────────
// 4. SPSC 数据完整性
// ──────────────────────────────────────────────────────────────
void TestSPSC() {
    TEST_SECTION("4. 单生产者单消费者（SPSC）");

    const int N = 100000;
    BoundedQueue<int> q;
    q.Init(1024);

    std::atomic<bool> done{false};
    std::vector<int> consumed;
    std::mutex consumed_mtx;

    std::thread producer([&]() {
        for (int i = 0; i < N; ++i) {
            while (!q.Enqueue(i)) { std::this_thread::yield(); }
        }
        done.store(true);
    });

    std::thread consumer([&]() {
        int val;
        while (!done.load() || !q.Empty()) {
            if (q.Dequeue(&val)) {
                std::lock_guard<std::mutex> lg(consumed_mtx);
                consumed.push_back(val);
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    TEST_ASSERT((int)consumed.size() == N, "消费数量 == 生产数量");

    // SPSC 下应该严格 FIFO
    bool ordered = true;
    for (int i = 0; i < N; ++i) {
        if (consumed[i] != i) { ordered = false; break; }
    }
    TEST_ASSERT(ordered, "SPSC 严格 FIFO 顺序");
}

// ──────────────────────────────────────────────────────────────
// 5. MPSC 数据完整性
// ──────────────────────────────────────────────────────────────
void TestMPSC() {
    TEST_SECTION("5. 多生产者单消费者（MPSC）");

    const int PRODUCERS = 4;
    const int PER_PRODUCER = 50000;
    const int TOTAL = PRODUCERS * PER_PRODUCER;

    BoundedQueue<int> q;
    q.Init(2048);

    std::atomic<bool> done{false};
    std::atomic<int> produced_count{0};

    // 每个生产者写入带标记的值：producer_id * 1000000 + seq
    std::vector<std::thread> producers;
    for (int p = 0; p < PRODUCERS; ++p) {
        producers.emplace_back([&, p]() {
            for (int i = 0; i < PER_PRODUCER; ++i) {
                int val = p * 1000000 + i;
                while (!q.Enqueue(val)) { std::this_thread::yield(); }
            }
            produced_count.fetch_add(1);
        });
    }

    std::vector<int> consumed;
    std::mutex consumed_mtx;

    std::thread consumer([&]() {
        int val;
        while (produced_count.load() < PRODUCERS || !q.Empty()) {
            if (q.Dequeue(&val)) {
                std::lock_guard<std::mutex> lg(consumed_mtx);
                consumed.push_back(val);
            } else {
                std::this_thread::yield();
            }
        }
    });

    for (auto& t : producers) t.join();
    consumer.join();

    TEST_ASSERT((int)consumed.size() == TOTAL, "消费数量 == 总生产数量");

    // 验证：每个生产者的值应该按序到达（单消费者保证了全局可见顺序）
    // 但不同生产者之间顺序不确定，所以只验证每个生产者自己的序列是否有序
    std::vector<std::vector<int>> per_producer(PRODUCERS);
    for (int val : consumed) {
        int pid = val / 1000000;
        int seq = val % 1000000;
        if (pid >= 0 && pid < PRODUCERS) {
            per_producer[pid].push_back(seq);
        }
    }
    bool each_ordered = true;
    for (int p = 0; p < PRODUCERS; ++p) {
        for (size_t i = 1; i < per_producer[p].size(); ++i) {
            if (per_producer[p][i] < per_producer[p][i - 1]) {
                each_ordered = false;
                break;
            }
        }
        TEST_ASSERT((int)per_producer[p].size() == PER_PRODUCER,
                     ("生产者 " + std::to_string(p) + " 的值全部到达").c_str());
    }
    TEST_ASSERT(each_ordered, "每个生产者内部有序（commit 保证）");
}

// ──────────────────────────────────────────────────────────────
// 6. SPMC 数据完整性
// ──────────────────────────────────────────────────────────────
void TestSPMC() {
    TEST_SECTION("6. 单生产者多消费者（SPMC）");

    const int CONSUMERS = 4;
    const int N = 100000;

    BoundedQueue<int> q;
    q.Init(2048);

    std::atomic<bool> done{false};
    std::vector<int> all_consumed;
    std::mutex all_mtx;

    std::thread producer([&]() {
        for (int i = 0; i < N; ++i) {
            while (!q.Enqueue(i)) { std::this_thread::yield(); }
        }
        done.store(true);
    });

    std::vector<std::thread> consumers;
    for (int c = 0; c < CONSUMERS; ++c) {
        consumers.emplace_back([&]() {
            int val;
            while (!done.load() || !q.Empty()) {
                if (q.Dequeue(&val)) {
                    std::lock_guard<std::mutex> lg(all_mtx);
                    all_consumed.push_back(val);
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }

    producer.join();
    for (auto& t : consumers) t.join();

    TEST_ASSERT((int)all_consumed.size() == N, "所有值都被消费");

    // 每个值恰好出现一次（无丢失、无重复）
    std::sort(all_consumed.begin(), all_consumed.end());
    bool unique_and_complete = true;
    for (int i = 0; i < N; ++i) {
        if (all_consumed[i] != i) { unique_and_complete = false; break; }
    }
    TEST_ASSERT(unique_and_complete, "每个值恰好消费一次（无丢失无重复）");
}

// ──────────────────────────────────────────────────────────────
// 7. MPMC 数据完整性
// ──────────────────────────────────────────────────────────────
void TestMPMC() {
    TEST_SECTION("7. 多生产者多消费者（MPMC）");

    const int PRODUCERS = 4;
    const int CONSUMERS = 4;
    const int PER_PRODUCER = 25000;
    const int TOTAL = PRODUCERS * PER_PRODUCER;

    BoundedQueue<int> q;
    q.Init(4096);

    std::atomic<int> produced_count{0};
    std::vector<int> all_consumed;
    std::mutex all_mtx;

    std::vector<std::thread> producers;
    for (int p = 0; p < PRODUCERS; ++p) {
        producers.emplace_back([&, p]() {
            for (int i = 0; i < PER_PRODUCER; ++i) {
                int val = p * PER_PRODUCER + i;
                while (!q.Enqueue(val)) { std::this_thread::yield(); }
            }
            produced_count.fetch_add(1);
        });
    }

    std::vector<std::thread> consumers;
    for (int c = 0; c < CONSUMERS; ++c) {
        consumers.emplace_back([&]() {
            int val;
            while (produced_count.load() < PRODUCERS || !q.Empty()) {
                if (q.Dequeue(&val)) {
                    std::lock_guard<std::mutex> lg(all_mtx);
                    all_consumed.push_back(val);
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    TEST_ASSERT((int)all_consumed.size() == TOTAL, "MPMC: 所有值都被消费");

    std::sort(all_consumed.begin(), all_consumed.end());
    bool complete = true;
    for (int i = 0; i < TOTAL; ++i) {
        if (all_consumed[i] != i) { complete = false; break; }
    }
    TEST_ASSERT(complete, "MPMC: 每个值恰好一次");
}

// ──────────────────────────────────────────────────────────────
// 8. WaitEnqueue / WaitDequeue 阻塞唤醒
// ──────────────────────────────────────────────────────────────
void TestWait() {
    TEST_SECTION("8. WaitEnqueue / WaitDequeue 阻塞唤醒");

    BoundedQueue<int> q;
    q.Init(4);

    const int N = 100;
    std::atomic<int> consumed_count{0};
    std::atomic<bool> done{false};

    // 消费者先启动，队列为空会阻塞
    std::thread consumer([&]() {
        int val;
        for (int i = 0; i < N; ++i) {
            q.WaitDequeue(&val);
            TEST_ASSERT(val == i, ("WaitDequeue 值: 期望 " + std::to_string(i) +
                         " 得到 " + std::to_string(val)).c_str());
            consumed_count.fetch_add(1);
        }
        done.store(true);
    });

    // 等消费者阻塞
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // 生产者写入，每写一个等消费者确认
    std::thread producer([&]() {
        for (int i = 0; i < N; ++i) {
            q.WaitEnqueue(i);
        }
    });

    producer.join();
    consumer.join();

    TEST_ASSERT(consumed_count.load() == N, "Wait 模式消费数量正确");
}

// ──────────────────────────────────────────────────────────────
// 9. BreakAllWait
// ──────────────────────────────────────────────────────────────
void TestBreakAllWait() {
    TEST_SECTION("9. BreakAllWait 中断等待");

    BoundedQueue<int> q;
    q.Init(4, new BlockWaitStrategy());

    std::atomic<bool> enqueue_returned{false};
    std::atomic<bool> enqueue_result{false};

    // 在满队列上 WaitEnqueue，会阻塞
    for (int i = 0; i < 4; ++i) q.Enqueue(i);

    std::thread waiter([&]() {
        enqueue_result.store(q.WaitEnqueue(999));
        enqueue_returned.store(true);
    });

    // 等线程阻塞
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    TEST_ASSERT(!enqueue_returned.load(), "WaitEnqueue 在满队列上阻塞中");

    // 中断
    q.BreakAllWait();
    waiter.join();

    TEST_ASSERT(enqueue_returned.load(), "BreakAllWait 后 WaitEnqueue 返回");
    TEST_ASSERT(!enqueue_result.load(), "BreakAllWait 后返回 false");

    // 同样测试 WaitDequeue
    BoundedQueue<int> q2;
    q2.Init(4, new BlockWaitStrategy());

    std::atomic<bool> dequeue_returned{false};
    std::atomic<bool> dequeue_result{false};

    std::thread waiter2([&]() {
        int val;
        dequeue_result.store(q2.WaitDequeue(&val));
        dequeue_returned.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    TEST_ASSERT(!dequeue_returned.load(), "WaitDequeue 在空队列上阻塞中");

    q2.BreakAllWait();
    waiter2.join();

    TEST_ASSERT(dequeue_returned.load(), "BreakAllWait 后 WaitDequeue 返回");
    TEST_ASSERT(!dequeue_result.load(), "BreakAllWait 后返回 false");
}

// ──────────────────────────────────────────────────────────────
// 10. 高并发压力测试
// ──────────────────────────────────────────────────────────────
void TestStress() {
    TEST_SECTION("10. 高并发压力测试（MPMC）");

    const int PRODUCERS = 8;
    const int CONSUMERS = 8;
    const int PER_PRODUCER = 100000;
    const int TOTAL = PRODUCERS * PER_PRODUCER;

    BoundedQueue<int> q;
    q.Init(8192);

    std::atomic<int> produced_count{0};
    std::vector<int> all_consumed;
    std::mutex all_mtx;

    auto t_start = std::chrono::steady_clock::now();

    std::vector<std::thread> producers;
    for (int p = 0; p < PRODUCERS; ++p) {
        producers.emplace_back([&, p]() {
            for (int i = 0; i < PER_PRODUCER; ++i) {
                int val = p * PER_PRODUCER + i;
                while (!q.Enqueue(val)) { std::this_thread::yield(); }
            }
            produced_count.fetch_add(1);
        });
    }

    std::vector<std::thread> consumers;
    for (int c = 0; c < CONSUMERS; ++c) {
        consumers.emplace_back([&]() {
            int val;
            while (produced_count.load() < PRODUCERS || !q.Empty()) {
                if (q.Dequeue(&val)) {
                    std::lock_guard<std::mutex> lg(all_mtx);
                    all_consumed.push_back(val);
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    auto t_end = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();

    TEST_ASSERT((int)all_consumed.size() == TOTAL, "压力测试: 数量正确");

    std::sort(all_consumed.begin(), all_consumed.end());
    bool complete = true;
    for (int i = 0; i < TOTAL; ++i) {
        if (all_consumed[i] != i) { complete = false; break; }
    }
    TEST_ASSERT(complete, "压力测试: 每个值恰好一次");

    std::cout << "  8P8C " << TOTAL << " ops, 耗时 " << (int)ms << " ms, "
              << (uint64_t)(TOTAL * 1000.0 / ms) << " ops/sec" << std::endl;
}

// ──────────────────────────────────────────────────────────────
// 11. 性能对比：BoundedQueue vs mutex+queue
// ──────────────────────────────────────────────────────────────

// BoundedQueue 适配器：统一 Push/Pop 接口，使用 Wait 版本配合 SleepWaitStrategy(1us)
template <typename T>
class BoundedQueueAdapter {
public:
    explicit BoundedQueueAdapter(size_t cap) {
        q_.Init(cap, new hnu::rcmw::base::SleepWaitStrategy(1));  // 1 微秒，而非默认 10ms
    }
    bool Push(const T& val) { return q_.WaitEnqueue(val); }
    bool Pop(T* val) { return q_.WaitDequeue(val); }
    bool TryPop(T* val) { return q_.Dequeue(val); }  // 非阻塞，用于尾部清理
    bool Empty() { return q_.Empty(); }
    void BreakAllWait() { q_.BreakAllWait(); }
private:
    BoundedQueue<T> q_;
};

// 互斥锁保护的普通队列
template <typename T>
class MutexQueue {
public:
    explicit MutexQueue(size_t /*cap*/) {}  // 保持接口一致
    bool Push(const T& val) {
        std::lock_guard<std::mutex> lg(mtx_);
        q_.push(val);
        return true;
    }
    bool Pop(T* val) {
        std::lock_guard<std::mutex> lg(mtx_);
        if (q_.empty()) return false;
        *val = q_.front();
        q_.pop();
        return true;
    }
    bool TryPop(T* val) { return Pop(val); }  // 接口一致
    bool Empty() {
        std::lock_guard<std::mutex> lg(mtx_);
        return q_.empty();
    }
    void BreakAllWait() {}  // 空操作，保持接口一致
private:
    std::queue<T> q_;
    std::mutex mtx_;
};

struct BenchResult {
    double ms;
    uint64_t ops;
    uint64_t ops_per_sec;
};

// 测试值工厂：根据类型生成合理的测试数据
template <typename T>
T MakeTestVal(int i) { return T(i); }  // int, LargeObject 都可以用 int 构造

template <>
std::string MakeTestVal<std::string>(int i) {
    return std::string(500, (char)(i % 128));  // 500B 字符串
}

// 通用基准测试：P 个生产者 C 个消费者，每人 N 个操作
template <typename QueueT, typename ValT>
BenchResult BenchMPMC(int P, int C, int N, size_t queue_cap) {
    QueueT q(queue_cap);
    std::atomic<int> produced_count{0};
    std::atomic<int> consumed_count{0};
    std::atomic<bool> start{false};

    auto t_start = std::chrono::steady_clock::now();

    std::vector<std::thread> producers;
    for (int p = 0; p < P; ++p) {
        producers.emplace_back([&, p]() {
            while (!start.load(std::memory_order_acquire)) {}
            for (int i = 0; i < N; ++i) {
                ValT val = MakeTestVal<ValT>(p * N + i);
                q.Push(val);  // WaitEnqueue 内部处理等待
            }
            produced_count.fetch_add(1);
        });
    }

    std::vector<std::thread> consumers;
    for (int c = 0; c < C; ++c) {
        consumers.emplace_back([&]() {
            while (!start.load(std::memory_order_acquire)) {}
            ValT val;
            while (produced_count.load() < P || !q.Empty()) {
                q.Pop(&val);        // WaitDequeue 内部处理等待
                consumed_count.fetch_add(1);
            }
        });
    }

    start.store(true, std::memory_order_release);
    for (auto& t : producers) t.join();
    // 先用非阻塞方式清空队列，再 BreakAllWait 唤醒可能阻塞的消费者
    ValT tmp;
    while (q.TryPop(&tmp)) { consumed_count.fetch_add(1); }
    q.BreakAllWait();
    for (auto& t : consumers) t.join();

    auto t_end = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    uint64_t total = consumed_count.load();
    return { ms, total, (uint64_t)(total * 1000.0 / ms) };
}

// 大对象：模拟通信中间件中消息体的拷贝开销
struct LargeObject {
    char data[1024];  // 1KB
    int id;
    LargeObject() : id(0) { memset(data, 0, sizeof(data)); }
    explicit LargeObject(int i) : id(i) { memset(data, i & 0xFF, sizeof(data)); }
};

void TestBenchmark() {
    TEST_SECTION("11. 性能对比：BoundedQueue vs MutexQueue");

    struct Scenario {
        std::string name;
        int P, C, N;
    };

    std::vector<Scenario> scenarios = {
        {"SPSC",         1,  1, 200000},
        {"MPSC(4P1C)",   4,  1, 100000},
        {"SPMC(1P4C)",   1,  4, 200000},
        {"MPMC(4P4C)",   4,  4, 100000},
        {"MPMC(8P8C)",   8,  8,  50000},
        {"MPMC(16P16C)",16, 16,  30000},
        {"MPMC(24P24C)",24, 24,  20000},
    };

    const size_t QUEUE_CAP = 4096;

    // ── int 小对象 ──
    std::cout << "\n  [小对象 int (4B)]" << std::endl;
    std::cout << std::left
              << std::setw(14) << "场景"
              << std::setw(16) << "BoundedQueue"
              << std::setw(16) << "MutexQueue"
              << std::setw(10) << "倍数"
              << std::endl;
    std::cout << std::string(56, '-') << std::endl;

    for (auto& sc : scenarios) {
        auto r1 = BenchMPMC<BoundedQueueAdapter<int>, int>(sc.P, sc.C, sc.N, QUEUE_CAP);
        auto r2 = BenchMPMC<MutexQueue<int>, int>(sc.P, sc.C, sc.N, QUEUE_CAP);
        double ratio = (r2.ops_per_sec > 0) ? (double)r1.ops_per_sec / r2.ops_per_sec : 0;
        std::cout << std::left
                  << std::setw(14) << sc.name
                  << std::setw(16) << r1.ops_per_sec
                  << std::setw(16) << r2.ops_per_sec
                  << std::fixed << std::setprecision(2) << ratio << "x"
                  << std::endl;
    }

    // ── LargeObject 大对象 ──
    std::cout << "\n  [大对象 LargeObject (1KB)]" << std::endl;
    std::cout << std::left
              << std::setw(14) << "场景"
              << std::setw(16) << "BoundedQueue"
              << std::setw(16) << "MutexQueue"
              << std::setw(10) << "倍数"
              << std::endl;
    std::cout << std::string(56, '-') << std::endl;

    for (auto& sc : scenarios) {
        auto r1 = BenchMPMC<BoundedQueueAdapter<LargeObject>, LargeObject>(sc.P, sc.C, sc.N, QUEUE_CAP);
        auto r2 = BenchMPMC<MutexQueue<LargeObject>, LargeObject>(sc.P, sc.C, sc.N, QUEUE_CAP);
        double ratio = (r2.ops_per_sec > 0) ? (double)r1.ops_per_sec / r2.ops_per_sec : 0;
        std::cout << std::left
                  << std::setw(14) << sc.name
                  << std::setw(16) << r1.ops_per_sec
                  << std::setw(16) << r2.ops_per_sec
                  << std::fixed << std::setprecision(2) << ratio << "x"
                  << std::endl;
    }

    // ── string 模拟真实消息 ──
    std::cout << "\n  [消息对象 std::string (~500B)]" << std::endl;
    std::cout << std::left
              << std::setw(14) << "场景"
              << std::setw(16) << "BoundedQueue"
              << std::setw(16) << "MutexQueue"
              << std::setw(10) << "倍数"
              << std::endl;
    std::cout << std::string(56, '-') << std::endl;

    for (auto& sc : scenarios) {
        auto r1 = BenchMPMC<BoundedQueueAdapter<std::string>, std::string>(sc.P, sc.C, sc.N, QUEUE_CAP);
        auto r2 = BenchMPMC<MutexQueue<std::string>, std::string>(sc.P, sc.C, sc.N, QUEUE_CAP);
        double ratio = (r2.ops_per_sec > 0) ? (double)r1.ops_per_sec / r2.ops_per_sec : 0;
        std::cout << std::left
                  << std::setw(14) << sc.name
                  << std::setw(16) << r1.ops_per_sec
                  << std::setw(16) << r2.ops_per_sec
                  << std::fixed << std::setprecision(2) << ratio << "x"
                  << std::endl;
    }

    std::cout << "\n  >1x 表示 BoundedQueue 更快，<1x 表示 MutexQueue 更快" << std::endl;
}

// ──────────────────────────────────────────────────────────────
// Main
// ──────────────────────────────────────────────────────────────
int main() {
    std::cout << "===== BoundedQueue 全面测试 =====" << std::endl;

    TestBasic();
    TestFull();
    TestEmpty();
    TestSPSC();
    TestMPSC();
    TestSPMC();
    TestMPMC();
    TestWait();
    TestBreakAllWait();
    TestStress();
    TestBenchmark();

    std::cout << "\n===== 结果 =====" << std::endl;
    std::cout << "通过: " << g_pass << "  失败: " << g_fail << std::endl;

    return g_fail > 0 ? 1 : 0;
}
