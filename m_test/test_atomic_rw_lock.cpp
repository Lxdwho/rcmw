#include <iostream>
#include <vector>
#include <thread>
#include "../base/atomic_rw_lock.h"

using namespace hnu::rcmw::base;

int main() {
    AtomicRWLock rwlock(false); // 启用写优先
    int shared_data = 0;

    auto reader = [&](int id) {
        for (int i = 0; i < 10; ++i) {
            ReadLockGuard<AtomicRWLock> rguard(rwlock);
            std::cout << "[Reader " << id << "] read value: " << shared_data << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    };

    auto writer = [&](int id) {
        for (int i = 0; i < 5; ++i) {
            WriteLockGuard<AtomicRWLock> wguard(rwlock);
            ++shared_data;
            std::cout << ">>> [Writer " << id << "] write value: " << shared_data << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    std::vector<std::thread> threads;

    // 启动多个读线程
    for (int i = 0; i < 8; ++i)
        threads.emplace_back(reader, i + 1);

    // 启动多个写线程
    for (int i = 0; i < 5; ++i)
        threads.emplace_back(writer, i + 1);

    for (auto& t : threads)
        t.join();

    std::cout << "Final shared_data = " << shared_data << std::endl;
    return 0;
}
