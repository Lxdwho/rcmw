#include <iostream>
#include <vector>
#include <thread>
#include "../base/unbounded_queue.h"

using namespace hnu::rcmw::base;

int main() {
    UnboundedQueue<int> que;

    auto enqueue = [&](int id) {
        for (int i = 0; i < 5; ++i) {
            que.Enqueue(1);
            std::cout << "[Enqueue " << id << "] Que.size() = " << que.Size() << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    };

    auto dequeue = [&](int id) {
        for (int i = 0; i < 5; ++i) {
            int* tmp = new int();
            que.Dequeue(tmp);
            std::cout << "[----Dequeue " << id << "] Que.size() = " << que.Size()
            << " head = " << *tmp << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    };

    std::vector<std::thread> threads;

    // 启动多个读线程
    for (int i = 0; i < 8; ++i)
        threads.emplace_back(enqueue, i + 1);

    // 启动多个写线程
    for (int i = 0; i < 5; ++i)
        threads.emplace_back(dequeue, i + 1);

    for (auto& t : threads)
        t.join();

    std::cout << "Final shared_data = " << que.Size() << std::endl;
    return 0;
}
