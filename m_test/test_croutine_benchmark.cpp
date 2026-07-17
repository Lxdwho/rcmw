/**
 * @brief  协程 vs 线程 性能对比
 *
 * 测试项目：
 *   1. 创建开销
 *   2. 切换开销（上下文切换）
 *   3. 并发调度开销
 *
 * 编译：make test_croutine_benchmark
 * 运行：./test_croutine_benchmark
 */

#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <functional>
#include <numeric>
#include <mutex>
#include <condition_variable>

#include "croutine/croutine.h"
#include "common/global_data.h"

using namespace hnu::rcmw::croutine;
using namespace hnu::rcmw::common;

// ──────────────────────────────────────────────────────────────
// 计时工具
// ──────────────────────────────────────────────────────────────
class Timer {
public:
    void Start() { start_ = std::chrono::steady_clock::now(); }
    void Stop()  { end_ = std::chrono::steady_clock::now(); }

    double ElapsedUs() const {
        return std::chrono::duration_cast<std::chrono::microseconds>(end_ - start_).count();
    }

    double ElapsedNs() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end_ - start_).count();
    }

private:
    std::chrono::steady_clock::time_point start_;
    std::chrono::steady_clock::time_point end_;
};

static void PrintResult(const std::string& name, int iterations, double total_ns) {
    double avg_ns = total_ns / iterations;
    std::cout << std::left << std::setw(35) << name
              << std::right
              << std::setw(10) << iterations << " 次"
              << std::setw(12) << std::fixed << std::setprecision(1) << total_ns / 1e6 << " ms"
              << std::setw(12) << std::fixed << std::setprecision(1) << avg_ns << " ns/次"
              << std::endl;
}

// ──────────────────────────────────────────────────────────────
// 测试 1：创建开销对比
// ──────────────────────────────────────────────────────────────
void BenchmarkCreate(int iterations) {
    std::cout << "\n========== 测试 1：创建开销 ==========" << std::endl;
    std::cout << std::left << std::setw(35) << "项目"
              << std::right
              << std::setw(14) << "次数"
              << std::setw(12) << "总耗时"
              << std::setw(12) << "平均"
              << std::endl;
    std::cout << std::string(73, '-') << std::endl;

    Timer timer;

    // 线程创建+销毁
    {
        timer.Start();
        for (int i = 0; i < iterations; i++) {
            std::thread t([]{});
            t.join();
        }
        timer.Stop();
        PrintResult("std::thread 创建+join", iterations, timer.ElapsedNs());
    }

    // 协程创建
    {
        timer.Start();
        for (int i = 0; i < iterations; i++) {
            CRoutine cr([]{});
        }
        timer.Stop();
        PrintResult("CRoutine 创建", iterations, timer.ElapsedNs());
    }
}

// ──────────────────────────────────────────────────────────────
// 测试 2：上下文切换开销对比
// ──────────────────────────────────────────────────────────────
void BenchmarkSwitch(int iterations) {
    std::cout << "\n========== 测试 2：上下文切换开销 ==========" << std::endl;
    std::cout << std::left << std::setw(35) << "项目"
              << std::right
              << std::setw(14) << "次数"
              << std::setw(12) << "总耗时"
              << std::setw(12) << "平均"
              << std::endl;
    std::cout << std::string(73, '-') << std::endl;

    Timer timer;

    // 线程上下文切换：通过条件变量交替唤醒
    {
        std::mutex mtx;
        std::condition_variable cv;
        bool ready = false;
        bool done = false;

        std::thread t([&]{
            std::unique_lock<std::mutex> lk(mtx);
            while (!done) {
                cv.wait(lk, [&]{ return ready || done; });
                if (done) break;
                ready = false;
                cv.notify_one();
            }
        });

        std::unique_lock<std::mutex> lk(mtx);
        timer.Start();
        for (int i = 0; i < iterations; i++) {
            ready = true;
            cv.notify_one();
            cv.wait(lk, [&]{ return !ready; });
        }
        timer.Stop();
        done = true;
        lk.unlock();
        cv.notify_one();
        t.join();
        PrintResult("std::thread 条件变量切换", iterations, timer.ElapsedNs());
    }

    // 协程上下文切换：Resume + Yield（复用同一个协程）
    {
        CRoutine cr([]{
            while (true) {
                CRoutine::Yield(RoutineState::IO_WAIT);
            }
        });

        timer.Start();
        for (int i = 0; i < iterations; i++) {
            cr.set_state(RoutineState::READY);
            cr.Resume();
        }
        timer.Stop();
        PrintResult("CRoutine Resume+Yield", iterations, timer.ElapsedNs());
    }
}

// ──────────────────────────────────────────────────────────────
// 测试 3：并发任务调度（大量任务，比完成速度）
// 线程方案：线程池（CPU核数），从共享队列取任务
// 协程方案：少量线程，每线程大量协程
// ──────────────────────────────────────────────────────────────
void BenchmarkConcurrent(int total_tasks) {
    std::cout << "\n========== 测试 3：并发任务调度 (" << total_tasks << " 个任务) ==========" << std::endl;
    std::cout << std::left << std::setw(35) << "项目"
              << std::right
              << std::setw(10) << "线程"
              << std::setw(10) << "协程"
              << std::setw(12) << "总耗时"
              << std::endl;
    std::cout << std::string(67, '-') << std::endl;

    Timer timer;
    std::atomic<uint64_t> counter{0};

    // 线程方案：100个线程，每个线程从原子计数器取任务
    {
        counter = 0;
        std::atomic<uint64_t> task_idx{0};
        int thread_num = 100;

        timer.Start();

        std::vector<std::thread> threads;
        for (int i = 0; i < thread_num; i++) {
            threads.emplace_back([&]{
                while (true) {
                    uint64_t idx = task_idx.fetch_add(1, std::memory_order_relaxed);
                    if (idx >= (uint64_t)total_tasks) break;
                    counter.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }
        for (auto& t : threads) t.join();

        timer.Stop();
        std::cout << std::left << std::setw(35) << "std::thread 线程池"
                  << std::right
                  << std::setw(8) << thread_num
                  << std::setw(10) << "-"
                  << std::setw(10) << std::fixed << std::setprecision(1)
                  << timer.ElapsedUs() << " us" << std::endl;
    }

    // 协程方案：4个线程，每线程25个协程，共100个协程，协程循环取任务
    {
        counter = 0;
        std::atomic<uint64_t> task_idx{0};
        int thread_num = 4;
        int routine_per_thread = 25;

        timer.Start();

        std::vector<std::thread> threads;
        for (int t = 0; t < thread_num; t++) {
            threads.emplace_back([&]{
                std::vector<std::unique_ptr<CRoutine>> routines;
                for (int j = 0; j < routine_per_thread; j++) {
                    routines.push_back(std::make_unique<CRoutine>([&]{
                        while (true) {
                            uint64_t idx = task_idx.fetch_add(1, std::memory_order_relaxed);
                            if (idx >= (uint64_t)total_tasks) break;
                            counter.fetch_add(1, std::memory_order_relaxed);
                            CRoutine::Yield(RoutineState::IO_WAIT);
                        }
                    }));
                }
                // 轮询调度直到所有任务完成
                while (task_idx.load() < (uint64_t)total_tasks) {
                    for (auto& cr : routines) {
                        if (cr->state() == RoutineState::IO_WAIT) {
                            cr->set_state(RoutineState::READY);
                        }
                        cr->Resume();
                    }
                }
            });
        }
        for (auto& t : threads) t.join();

        timer.Stop();
        std::cout << std::left << std::setw(35) << "CRoutine 协程调度"
                  << std::right
                  << std::setw(8) << thread_num
                  << std::setw(10) << routine_per_thread * thread_num
                  << std::setw(10) << std::fixed << std::setprecision(1)
                  << timer.ElapsedUs() << " us" << std::endl;
    }
}

// ──────────────────────────────────────────────────────────────
// 测试 4：内存开销对比
// ──────────────────────────────────────────────────────────────
void BenchmarkMemory() {
    std::cout << "\n========== 测试 4：单个对象内存开销 ==========" << std::endl;
    std::cout << std::left << std::setw(35) << "项目"
              << std::right
              << std::setw(14) << "大小"
              << std::endl;
    std::cout << std::string(49, '-') << std::endl;

    std::cout << std::left << std::setw(35) << "std::thread"
              << std::right
              << std::setw(10) << sizeof(std::thread) << " 字节"
              << std::endl;

    std::cout << std::left << std::setw(35) << "CRoutine (不含栈)"
              << std::right
              << std::setw(10) << sizeof(CRoutine) << " 字节"
              << std::endl;

    std::cout << std::left << std::setw(35) << "RoutineContext (含 2MB 栈)"
              << std::right
              << std::setw(10) << sizeof(RoutineContext) << " 字节"
              << std::endl;

    std::cout << std::left << std::setw(35) << "CRoutine + RoutineContext 总计"
              << std::right
              << std::setw(10) << sizeof(CRoutine) + sizeof(RoutineContext) << " 字节"
              << std::endl;

    // 线程栈大小（默认）
    std::cout << std::left << std::setw(35) << "std::thread 默认栈"
              << std::right
              << std::setw(10) << "8192" << " 字节 (8MB, ulimit -s)"
              << std::endl;
}

// ──────────────────────────────────────────────────────────────
// 主函数
// ──────────────────────────────────────────────────────────────
int main() {
    Logger_Init("test_croutine_benchmark.log");
    GlobalData::Instance();
    // 设置足够大的对象池，避免 benchmark 中耗尽
    // 每个 RoutineContext 约 2MB，按实际测试数量设置
    GlobalData::Instance()->SetComponentNums(1500);

    std::cout << "============================================================" << std::endl;
    std::cout << "  协程 vs 线程 性能对比" << std::endl;
    std::cout << "============================================================" << std::endl;

    const int SMALL  = 100;
    const int MEDIUM = 500;
    const int LARGE  = 5000;

    BenchmarkCreate(MEDIUM);
    BenchmarkSwitch(LARGE);
    BenchmarkConcurrent(1000);  // 1000个任务
    BenchmarkMemory();

    std::cout << "\n============================================================" << std::endl;
    std::cout << "  测试完成" << std::endl;
    std::cout << "============================================================" << std::endl;

    return 0;
}
