#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <chrono>
#include <cassert>
#include "atomic_hash_map.h"

void process(int &  x) { std::cout << "左值函数触发: " << x << std::endl;}
void process(int && x) { std::cout << "右值函数触发: " << x << std::endl;}

template <class T>
void forwardExample(T && arg) {
    process(std::forward<T>(arg));
}

template <class T>
class Test {
public:
    void forwardExample(T && arg) {
        process(std::forward<T>(arg));
    }

    void forwardExample1(T && arg) {
        process(std::move<T>(arg));
    }
};

using namespace hnu::rcmw::base;

int main()
{
    // int a = 10;  // a是左值
    // int b = std::move(a); // 右值引用

    // std::string as = "Hello";
    // std::string bs = std::move(as);

    // std::cout << a << " " << b << std::endl;
    // std::cout << as << " " << bs << std::endl;


    // int a = 10;
    // forwardExample(a);
    // forwardExample(20);

    AtomicHashMap<int, int> map;
    map.Set(2, 4);
    map.Set(1, 3);
    map.Set(8, 7);
    map.Set(12, 5);
    map.Set(2, 8);
    int a, b, c, d, e;
    map.Get(2, &a);
    map.Get(1, &b);
    map.Get(8, &c);
    map.Get(12, &d);
    std::cout << a << " "
              << b << " "
              << c << " "
              << d << " "
              << std::endl;
}

/*
// @brief  演示 AtomicHashMap::Get(key, V**) 返回裸指针的悬垂风险
// 
//  场景：
//    线程 A: Get(key, &ptr)  → 拿到 V* 指针
//    线程 B: Set(key, new_v) → CAS 替换 value_ptr，delete 旧值
//    线程 A: 解引用 ptr      → use-after-free (未定义行为)
// 
//  编译：
//    g++ -std=c++14 -pthread -g -fsanitize=address test_get_dangling_ptr.cpp -o test_get_dangling_ptr
//  运行：
//    ./test_get_dangling_ptr
// 
//  预期结果（AddressSanitizer 开启时）：
//    输出 ==DETECTED== 表示确实触发了 use-after-free
//    或者输出 OK 说明本次运行未触发（概率性，可增大 kIterations）
// 

using hnu::rcmw::base::AtomicHashMap;

// 用 string 作为 value，延长拷贝/析构时间，放大竞争窗口
using Map = AtomicHashMap<int, std::string, 64>;

static constexpr int kKey        = 42;
static constexpr int kIterations = 200000;

int main()
{
    Map map;

    // 先插入一个初始值
    map.Set(kKey, std::string("init"));

    std::atomic<bool> stop{false};
    std::atomic<int>  dangling_detected{0};

    // --- 线程 A：反复 Get，拿到指针后立即解引用 ---
    auto reader = [&]() {
        while (!stop.load(std::memory_order_relaxed)) {
            std::string ptr;
            if (map.Get(kKey, &ptr)) {
                // 模拟"拿到指针后做一点事"的延迟，
                // 让线程 B 有时间 delete 掉这块内存
                std::string copy;
                // 分多步访问，拉长窗口
                copy = ptr;               // ← 这里可能已经 UAF
                if (copy.empty()) {
                    // 不应该走到这里，empty 不是合法状态
                    dangling_detected.fetch_add(1, std::memory_order_relaxed);
                }
            }
        }
    };

    // --- 线程 B：反复用新值覆盖同一个 key ---
    auto writer = [&]() {
        for (int i = 0; i < kIterations; ++i) {
            map.Set(kKey, std::string("value_" + std::to_string(i)));
        }
        stop.store(true, std::memory_order_relaxed);
    };

    std::thread t_writer(writer);
    std::thread t_reader(reader);

    t_writer.join();
    t_reader.join();

    if (dangling_detected.load() > 0) {
        std::cout << "==DETECTED==  检测到 " << dangling_detected.load()
                  << " 次可能的 use-after-free 访问" << std::endl;
    } else {
        std::cout << "OK  本次运行未触发崩溃（不等于安全，"
                  << "请用 -fsanitize=address 编译后重试）" << std::endl;
    }

    // 最终读一次确认 map 状态
    std::string final_val;
    if (map.Get(kKey, &final_val)) {
        std::cout << "最终值: " << final_val << std::endl;
    }

    return 0;
}
*/
