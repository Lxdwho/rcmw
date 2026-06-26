/**
 * @brief  UnboundedQueue 测试
 *
 * 队列设计为 MPSC（多生产者单消费者），多消费者需要外部 mutex 串行化
 *
 * 编译（ASan 检测内存问题）：
 *   g++ -std=c++14 -pthread -g -fsanitize=address -I . m_test/test_unbounded_queue.cpp -o test_unbounded_queue
 *
 * 编译（正常模式）：
 *   g++ -std=c++14 -pthread -O2 -I . m_test/test_unbounded_queue.cpp -o test_unbounded_queue
 *
 * 运行：./test_unbounded_queue
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <string>

#include "base/unbounded_queue.h"

using hnu::rcmw::base::UnboundedQueue;

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

    UnboundedQueue<int> q;

    TEST_ASSERT(q.Empty(), "初始为空");
    TEST_ASSERT(q.Size() == 0, "初始 Size=0");

    q.Enqueue(42);
    TEST_ASSERT(!q.Empty(), "入队后非空");
    TEST_ASSERT(q.Size() == 1, "Size=1");

    int val = 0;
    TEST_ASSERT(q.Dequeue(&val), "Dequeue 成功");
    TEST_ASSERT(val == 42, "值正确");
    TEST_ASSERT(q.Empty(), "取出后为空");

    // 批量 FIFO
    for (int i = 0; i < 100; ++i) q.Enqueue(i);
    for (int i = 0; i < 100; ++i) {
        q.Dequeue(&val);
        TEST_ASSERT(val == i, ("FIFO: 期望 " + std::to_string(i) + " 得到 " + std::to_string(val)).c_str());
    }
}

// ──────────────────────────────────────────────────────────────
// 2. 空队列 Dequeue
// ──────────────────────────────────────────────────────────────
void TestEmptyDequeue() {
    TEST_SECTION("2. 空队列 Dequeue");

    UnboundedQueue<int> q;
    int val = -1;
    TEST_ASSERT(!q.Dequeue(&val), "空队列返回 false");
    TEST_ASSERT(val == -1, "val 未被修改");

    q.Enqueue(1);
    q.Dequeue(&val);
    TEST_ASSERT(!q.Dequeue(&val), "取完后再次 Dequeue 返回 false");
}

// ──────────────────────────────────────────────────────────────
// 3. SPSC 数据完整性
// ──────────────────────────────────────────────────────────────
void TestSPSC() {
    TEST_SECTION("3. 单生产者单消费者（SPSC）");

    const int N = 100000;
    UnboundedQueue<int> q;
    std::atomic<bool> done{false};
    std::vector<int> consumed;
    std::mutex mtx;

    std::thread producer([&]() {
        for (int i = 0; i < N; ++i) q.Enqueue(i);
        done.store(true);
    });

    std::thread consumer([&]() {
        int val;
        while (!done.load() || !q.Empty()) {
            if (q.Dequeue(&val)) {
                std::lock_guard<std::mutex> lg(mtx);
                consumed.push_back(val);
            }
        }
    });

    producer.join();
    consumer.join();

    TEST_ASSERT((int)consumed.size() == N, "消费数量正确");
    bool ordered = true;
    for (int i = 0; i < N; ++i) {
        if (consumed[i] != i) { ordered = false; break; }
    }
    TEST_ASSERT(ordered, "SPSC FIFO 顺序");
}

// ──────────────────────────────────────────────────────────────
// 4. 多生产者多消费者（Dequeue 用 mutex 串行化）
// ──────────────────────────────────────────────────────────────
void TestMultiConsumer() {
    TEST_SECTION("4. 多生产者多消费者（Dequeue mutex 串行化）");

    const int PRODUCERS = 2;
    const int CONSUMERS = 4;
    const int PER_PRODUCER = 50000;
    const int TOTAL = PRODUCERS * PER_PRODUCER;

    UnboundedQueue<int> q;
    std::atomic<int> produced_count{0};
    std::vector<int> consumed;
    std::mutex mtx;       // 保护 consumed
    std::mutex dequeue_mtx;  // 串行化 Dequeue

    std::vector<std::thread> producers;
    for (int p = 0; p < PRODUCERS; ++p) {
        producers.emplace_back([&, p]() {
            for (int i = 0; i < PER_PRODUCER; ++i) {
                q.Enqueue(p * PER_PRODUCER + i);
            }
            produced_count.fetch_add(1);
        });
    }

    std::vector<std::thread> consumers;
    for (int c = 0; c < CONSUMERS; ++c) {
        consumers.emplace_back([&]() {
            int val;
            while (produced_count.load() < PRODUCERS || !q.Empty()) {
                bool got = false;
                {
                    std::lock_guard<std::mutex> lg(dequeue_mtx);
                    got = q.Dequeue(&val);
                }
                if (got) {
                    std::lock_guard<std::mutex> lg(mtx);
                    consumed.push_back(val);
                }
            }
        });
    }

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    TEST_ASSERT((int)consumed.size() == TOTAL, "消费数量正确");
}

// ──────────────────────────────────────────────────────────────
// 5. 高并发压力测试（Dequeue mutex 串行化）
// ──────────────────────────────────────────────────────────────
void TestStress() {
    TEST_SECTION("5. 高并发压力测试（MP + mutex-SC）");

    const int PRODUCERS = 8;
    const int CONSUMERS = 4;
    const int PER_PRODUCER = 50000;
    const int TOTAL = PRODUCERS * PER_PRODUCER;

    UnboundedQueue<int> q;
    std::atomic<int> produced_count{0};
    std::atomic<int> consumed_count{0};
    std::mutex dequeue_mtx;

    auto t_start = std::chrono::steady_clock::now();

    std::vector<std::thread> producers;
    for (int p = 0; p < PRODUCERS; ++p) {
        producers.emplace_back([&, p]() {
            for (int i = 0; i < PER_PRODUCER; ++i) {
                q.Enqueue(p * PER_PRODUCER + i);
            }
            produced_count.fetch_add(1);
        });
    }

    std::vector<std::thread> consumers;
    for (int c = 0; c < CONSUMERS; ++c) {
        consumers.emplace_back([&]() {
            int val;
            while (produced_count.load() < PRODUCERS || !q.Empty()) {
                bool got;
                {
                    std::lock_guard<std::mutex> lg(dequeue_mtx);
                    got = q.Dequeue(&val);
                }
                if (got) consumed_count.fetch_add(1);
            }
        });
    }

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    auto t_end = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();

    std::cout << "  8P" << CONSUMERS << "C " << consumed_count.load() << "/" << TOTAL
              << " ops, " << (int)ms << " ms, "
              << (uint64_t)(consumed_count.load() * 1000.0 / ms) << " ops/sec" << std::endl;

    TEST_ASSERT(consumed_count.load() == TOTAL, "压力测试: 数量正确");
}

// ──────────────────────────────────────────────────────────────
// 6. 重复压力测试（MPSC 模式）
// ──────────────────────────────────────────────────────────────
void TestRepeatStress() {
    TEST_SECTION("6. 重复压力测试（10 轮 MPSC）");

    for (int round = 0; round < 10; ++round) {
        UnboundedQueue<int> q;
        const int ITEMS = 20000;
        std::atomic<int> consumed_count{0};
        std::mutex dequeue_mtx;

        std::vector<std::thread> producers;
        for (int p = 0; p < 4; ++p) {
            producers.emplace_back([&, p]() {
                for (int i = 0; i < ITEMS / 4; ++i) {
                    q.Enqueue(p * (ITEMS / 4) + i);
                }
            });
        }

        // 单消费者
        std::thread consumer([&]() {
            int val;
            while (consumed_count.load() < ITEMS) {
                if (q.Dequeue(&val)) consumed_count.fetch_add(1);
            }
        });

        for (auto& t : producers) t.join();
        consumer.join();

        std::cout << "  轮次 " << round + 1 << ": " << consumed_count.load() << " ops" << std::endl;
        TEST_ASSERT(consumed_count.load() == ITEMS, ("轮次 " + std::to_string(round + 1) + " 数量正确").c_str());
    }
}

// ──────────────────────────────────────────────────────────────
// 性能对比：UnboundedQueue vs MutexQueue（MPSC 模式）
// ──────────────────────────────────────────────────────────────

// 测试值工厂
template <typename T>
T MakeTestVal(int i) { return T(i); }

template <>
std::string MakeTestVal<std::string>(int i) {
    return std::string(500, (char)(i % 128));
}

// 互斥锁队列（单消费者 + 多生产者）
template <typename T>
class MutexQueueMPSC {
public:
    void Push(const T& val) {
        std::lock_guard<std::mutex> lg(mtx_);
        q_.push(val);
    }
    bool Pop(T* val) {
        std::lock_guard<std::mutex> lg(mtx_);
        if(q_.empty()) return false;
        *val = q_.front();
        q_.pop();
        return true;
    }
    bool Empty() {
        std::lock_guard<std::mutex> lg(mtx_);
        return q_.empty();
    }
private:
    std::queue<T> q_;
    std::mutex mtx_;
};

struct BenchResult {
    double ms;
    uint64_t ops;
    uint64_t ops_per_sec;
};

// MPSC 基准：P 个生产者 + 1 个消费者
// enqueue_fn: void(int idx)  — 根据 idx 构造值并入队
// dequeue_fn: bool(T* val)   — 出队
// empty_fn:   bool()         — 队列是否为空
template <typename EnqueueT, typename DequeueT, typename EmptyT, typename ValT>
BenchResult BenchMPSC(int P, int N,
                      EnqueueT enqueue_fn, DequeueT dequeue_fn, EmptyT empty_fn) {
    std::atomic<int> produced_count{0};
    std::atomic<int> consumed_count{0};
    std::atomic<bool> start{false};

    auto t_start = std::chrono::steady_clock::now();

    std::vector<std::thread> producers;
    for(int p = 0; p < P; ++p) {
        producers.emplace_back([&, p]() {
            while(!start.load(std::memory_order_acquire)) {}
            for(int i = 0; i < N; ++i) {
                enqueue_fn(p * N + i);
            }
            produced_count.fetch_add(1);
        });
    }

    std::thread consumer([&]() {
        while(!start.load(std::memory_order_acquire)) {}
        ValT val;
        while(produced_count.load() < P || !empty_fn()) {
            if(dequeue_fn(&val)) consumed_count.fetch_add(1);
        }
    });

    start.store(true, std::memory_order_release);
    for(auto& t : producers) t.join();
    consumer.join();

    auto t_end = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    uint64_t total = consumed_count.load();
    return { ms, total, (uint64_t)(total * 1000.0 / ms) };
}

void TestBenchmark() {
    TEST_SECTION("7. 性能对比：UnboundedQueue vs MutexQueue（MPSC）");

    struct Scenario {
        std::string name;
        int P, N;
    };

    std::vector<Scenario> scenarios = {
        {"1P1C",   1, 500000},
        {"2P1C",   2, 300000},
        {"4P1C",   4, 200000},
        {"8P1C",   8, 100000},
        {"16P1C", 16,  50000},
    };

    // ── int 小对象 ──
    std::cout << "\n  [小对象 int (4B)]" << std::endl;
    std::cout << std::left
              << std::setw(10) << "场景"
              << std::setw(16) << "UnboundedQueue"
              << std::setw(16) << "MutexQueue"
              << std::setw(10) << "倍数"
              << std::endl;
    std::cout << std::string(52, '-') << std::endl;

    for(auto& sc : scenarios) {
        UnboundedQueue<int> uq;
        MutexQueueMPSC<int> mq;

        auto enq1 = [&](int idx) { uq.Enqueue(MakeTestVal<int>(idx)); };
        auto deq1 = [&](int* v) { return uq.Dequeue(v); };
        auto emp1 = [&]() { return uq.Empty(); };
        auto enq2 = [&](int idx) { mq.Push(MakeTestVal<int>(idx)); };
        auto deq2 = [&](int* v) { return mq.Pop(v); };
        auto emp2 = [&]() { return mq.Empty(); };

        auto r1 = BenchMPSC<decltype(enq1), decltype(deq1), decltype(emp1), int>(
            sc.P, sc.N, enq1, deq1, emp1);
        auto r2 = BenchMPSC<decltype(enq2), decltype(deq2), decltype(emp2), int>(
            sc.P, sc.N, enq2, deq2, emp2);

        double ratio = (r2.ops_per_sec > 0) ? (double)r1.ops_per_sec / r2.ops_per_sec : 0;
        std::cout << std::left
                  << std::setw(10) << sc.name
                  << std::setw(16) << r1.ops_per_sec
                  << std::setw(16) << r2.ops_per_sec
                  << std::fixed << std::setprecision(2) << ratio << "x"
                  << std::endl;
    }

    // ── string 消息对象 ──
    std::cout << "\n  [消息对象 std::string (~500B)]" << std::endl;
    std::cout << std::left
              << std::setw(10) << "场景"
              << std::setw(16) << "UnboundedQueue"
              << std::setw(16) << "MutexQueue"
              << std::setw(10) << "倍数"
              << std::endl;
    std::cout << std::string(52, '-') << std::endl;

    for(auto& sc : scenarios) {
        UnboundedQueue<std::string> uq;
        MutexQueueMPSC<std::string> mq;

        auto enq1 = [&](int idx) { uq.Enqueue(MakeTestVal<std::string>(idx)); };
        auto deq1 = [&](std::string* v) { return uq.Dequeue(v); };
        auto emp1 = [&]() { return uq.Empty(); };
        auto enq2 = [&](int idx) { mq.Push(MakeTestVal<std::string>(idx)); };
        auto deq2 = [&](std::string* v) { return mq.Pop(v); };
        auto emp2 = [&]() { return mq.Empty(); };

        auto r1 = BenchMPSC<decltype(enq1), decltype(deq1), decltype(emp1), std::string>(
            sc.P, sc.N, enq1, deq1, emp1);
        auto r2 = BenchMPSC<decltype(enq2), decltype(deq2), decltype(emp2), std::string>(
            sc.P, sc.N, enq2, deq2, emp2);

        double ratio = (r2.ops_per_sec > 0) ? (double)r1.ops_per_sec / r2.ops_per_sec : 0;
        std::cout << std::left
                  << std::setw(10) << sc.name
                  << std::setw(16) << r1.ops_per_sec
                  << std::setw(16) << r2.ops_per_sec
                  << std::fixed << std::setprecision(2) << ratio << "x"
                  << std::endl;
    }

    std::cout << "\n  >1x 表示 UnboundedQueue 更快" << std::endl;
}

// ──────────────────────────────────────────────────────────────
// Main
// ──────────────────────────────────────────────────────────────
int main() {
    std::cout << "===== UnboundedQueue 测试 =====" << std::endl;

    TestBasic();
    TestEmptyDequeue();
    TestSPSC();
    TestMultiConsumer();
    TestStress();
    TestRepeatStress();
    TestBenchmark();

    std::cout << "\n===== 结果 =====" << std::endl;
    std::cout << "通过: " << g_pass << "  失败: " << g_fail << std::endl;
    std::cout << "\n提示：用 ASan 编译可精确捕获 double free：" << std::endl;
    std::cout << "  g++ -std=c++14 -pthread -g -fsanitize=address -I . m_test/test_unbounded_queue.cpp -o test_unbounded_queue" << std::endl;

    return g_fail > 0 ? 1 : 0;
}
