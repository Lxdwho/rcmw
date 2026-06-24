/**
 * @brief  互斥锁 vs 项目原子读写锁 vs 自旋锁 性能对比
 *
 * 编译：g++ -std=c++14 -pthread -O2 -I . m_test/test_lock_benchmark.cpp -o test_lock_benchmark
 * 运行：./test_lock_benchmark
 */

#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <string>
#include <functional>

#include "base/atomic_rw_lock.h"
#include "base/rw_lock_guard.h"

using hnu::rcmw::base::AtomicRWLock;
using hnu::rcmw::base::ReadLockGuard;
using hnu::rcmw::base::WriteLockGuard;

// ──────────────────────────────────────────────────────────────
// 自旋锁
// ──────────────────────────────────────────────────────────────
class SpinLock {
public:
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {}
    }
    void unlock() { flag_.clear(std::memory_order_release); }
private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
};

// ──────────────────────────────────────────────────────────────
// 锁包装器：统一接口
// ──────────────────────────────────────────────────────────────
struct MutexWrapper {
    std::mutex mtx;
    void ReadLock()    { mtx.lock(); }
    void ReadUnlock()  { mtx.unlock(); }
    void WriteLock()   { mtx.lock(); }
    void WriteUnlock() { mtx.unlock(); }
};

struct SpinLockWrapper {
    SpinLock spin;
    void ReadLock()    { spin.lock(); }
    void ReadUnlock()  { spin.unlock(); }
    void WriteLock()   { spin.lock(); }
    void WriteUnlock() { spin.unlock(); }
};

// 项目的 AtomicRWLock 直接使用，不需要包装

// ──────────────────────────────────────────────────────────────
// 基准测试
// ──────────────────────────────────────────────────────────────
struct BenchResult {
    double total_ms;
    uint64_t ops;
    double ops_per_sec;
};

/**
 * @param read_ratio   读占比 (0.0~1.0)
 * @param critical_spin 忙等次数（模拟临界区耗时）
 */
template <typename LockT>
BenchResult RunBench(int num_threads, double read_ratio, int critical_spin)
{
    LockT lock;
    uint64_t shared_data = 0;
    std::atomic<bool> start{false};
    std::atomic<bool> stop{false};
    std::atomic<uint64_t> op_count{0};

    auto worker = [&](int tid) {
        while (!start.load(std::memory_order_acquire)) {}

        uint64_t local_ops = 0;
        // xorshift64 随机数，每个线程独立种子
        uint64_t rng = (uint64_t)(tid + 1) * 2654435761ULL;
        auto next_rng = [&]() {
            rng ^= rng << 13;
            rng ^= rng >> 7;
            rng ^= rng << 17;
            return rng;
        };
        uint32_t threshold = (uint32_t)(read_ratio * 1000);

        while (!stop.load(std::memory_order_relaxed)) {
            bool do_read = (next_rng() % 1000) < threshold;

            if (do_read) {
                lock.ReadLock();
                volatile uint64_t tmp = shared_data;
                for (int i = 0; i < critical_spin; ++i) tmp += i;
                (void)tmp;
                lock.ReadUnlock();
            } else {
                lock.WriteLock();
                ++shared_data;
                for (int i = 0; i < critical_spin; ++i) shared_data += i;
                lock.WriteUnlock();
            }
            ++local_ops;
        }
        op_count.fetch_add(local_ops, std::memory_order_relaxed);
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i)
        threads.emplace_back(worker, i);

    auto t0 = std::chrono::steady_clock::now();
    start.store(true, std::memory_order_release);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    stop.store(true, std::memory_order_release);
    for (auto &t : threads) t.join();

    auto t1 = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    uint64_t ops = op_count.load();
    return { ms, ops, ops * 1000.0 / ms };
}

// AtomicRWLock 特化：使用项目的 ReadLockGuard / WriteLockGuard
template <>
BenchResult RunBench<AtomicRWLock>(int num_threads, double read_ratio, int critical_spin)
{
    AtomicRWLock lock(false);
    uint64_t shared_data = 0;
    std::atomic<bool> start{false};
    std::atomic<bool> stop{false};
    std::atomic<uint64_t> op_count{0};

    auto worker = [&](int tid) {
        while (!start.load(std::memory_order_acquire)) {}

        uint64_t local_ops = 0;
        uint64_t rng = (uint64_t)(tid + 1) * 2654435761ULL;
        auto next_rng = [&]() {
            rng ^= rng << 13;
            rng ^= rng >> 7;
            rng ^= rng << 17;
            return rng;
        };
        uint32_t threshold = (uint32_t)(read_ratio * 1000);

        while (!stop.load(std::memory_order_relaxed)) {
            bool do_read = (next_rng() % 1000) < threshold;

            if (do_read) {
                ReadLockGuard<AtomicRWLock> g(lock);
                volatile uint64_t tmp = shared_data;
                for (int i = 0; i < critical_spin; ++i) tmp += i;
                (void)tmp;
            } else {
                WriteLockGuard<AtomicRWLock> g(lock);
                ++shared_data;
                for (int i = 0; i < critical_spin; ++i) shared_data += i;
            }
            ++local_ops;
        }
        op_count.fetch_add(local_ops, std::memory_order_relaxed);
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i)
        threads.emplace_back(worker, i);

    auto t0 = std::chrono::steady_clock::now();
    start.store(true, std::memory_order_release);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    stop.store(true, std::memory_order_release);
    for (auto &t : threads) t.join();

    auto t1 = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    uint64_t ops = op_count.load();
    return { ms, ops, ops * 1000.0 / ms };
}

// ──────────────────────────────────────────────────────────────
// 输出
// ──────────────────────────────────────────────────────────────
void PrintHeader() {
    std::cout << std::left
              << std::setw(14) << "Lock"
              << std::setw(8)  << "Threads"
              << std::setw(10) << "R:W"
              << std::setw(14) << "Critical"
              << std::right
              << std::setw(14) << "Ops/sec"
              << std::setw(10) << "Time(ms)"
              << std::endl;
    std::cout << std::string(70, '-') << std::endl;
}

void PrintRow(const std::string &name, int threads,
              const std::string &rw, const std::string &crit,
              const BenchResult &r) {
    std::cout << std::left
              << std::setw(14) << name
              << std::setw(8)  << threads
              << std::setw(10) << rw
              << std::setw(14) << crit
              << std::right
              << std::setw(14) << (uint64_t)r.ops_per_sec
              << std::setw(10) << (int)r.total_ms
              << std::endl;
}

// ──────────────────────────────────────────────────────────────
// Main
// ──────────────────────────────────────────────────────────────
int main()
{
    std::cout << "\n===== 锁性能基准测试 =====" << std::endl;
    std::cout << "每组测试运行 2 秒，统计总操作次数\n" << std::endl;

    struct Scenario {
        int threads;
        double read_ratio;
        int spin;
        std::string rw_label;
        std::string crit_label;
    };

    std::vector<Scenario> scenarios = {
        // ── 无竞争基准 ──
        {1,  1.0,   0, "100:0",   "短(0)"},
        // ── 读多写少 + 短临界区 ──
        {4,  0.9,   0, "90:10",   "短(0)"},
        {8,  0.9,   0, "90:10",   "短(0)"},
        {16, 0.9,   0, "90:10",   "短(0)"},
        // ── 读多写少 + 长临界区 ──
        {4,  0.9, 100, "90:10",   "长(100)"},
        {8,  0.9, 100, "90:10",   "长(100)"},
        // ── 读写均衡 + 短临界区 ──
        {4,  0.5,   0, "50:50",   "短(0)"},
        {8,  0.5,   0, "50:50",   "短(0)"},
        {16, 0.5,   0, "50:50",   "短(0)"},
        // ── 读写均衡 + 长临界区 ──
        {4,  0.5, 100, "50:50",   "长(100)"},
        {8,  0.5, 100, "50:50",   "长(100)"},
    };

    for (auto &sc : scenarios) {
        std::cout << "\n--- 线程=" << sc.threads
                  << "  读写=" << sc.rw_label
                  << "  临界区=" << sc.crit_label << " ---" << std::endl;
        PrintHeader();

        auto r1 = RunBench<MutexWrapper>(sc.threads, sc.read_ratio, sc.spin);
        PrintRow("mutex", sc.threads, sc.rw_label, sc.crit_label, r1);

        auto r2 = RunBench<SpinLockWrapper>(sc.threads, sc.read_ratio, sc.spin);
        PrintRow("spinlock", sc.threads, sc.rw_label, sc.crit_label, r2);

        auto r3 = RunBench<AtomicRWLock>(sc.threads, sc.read_ratio, sc.spin);
        PrintRow("AtomicRWLock", sc.threads, sc.rw_label, sc.crit_label, r3);

        double mutex_ops = r1.ops_per_sec;
        if (mutex_ops > 0) {
            std::cout << "  → spinlock/mutex: " << std::fixed << std::setprecision(1)
                      << (r2.ops_per_sec / mutex_ops * 100) << "%"
                      << "  AtomicRWLock/mutex: "
                      << (r3.ops_per_sec / mutex_ops * 100) << "%" << std::endl;
        }
    }

    std::cout << "\n说明：" << std::endl;
    std::cout << "  短临界区(0) = 临界区内只做计数，耗时~10ns" << std::endl;
    std::cout << "  长临界区(100) = 临界区内忙等100次循环，耗时~200ns" << std::endl;
    std::cout << "  AtomicRWLock 在纯读或读多写少时应明显优于 mutex/spinlock" << std::endl;
    std::cout << "  spinlock 在无竞争短临界区时应与 mutex 持平，高竞争时因忙等可能略差" << std::endl;

    return 0;
}
