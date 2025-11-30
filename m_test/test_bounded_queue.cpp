#include <iostream>
#include <vector>
#include <thread>
#include "../base/bounded_queue.h"

using namespace hnu::rcmw::base;

int main() {
    BoundedQueue<int> que;
    que.Init(100);

    auto enqueue = [&](int id) {
        for (int i = 0; i < 5; ++i) {
            que.Enqueue(1);
            std::cout << "[Enqueue " << id << "] Que.size() = " << que.Size() << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    };

    auto waitenque = [&](int id) {
        for (int i = 0; i < 5; ++i) {
            int tmp;
            que.WaitEnqueue(tmp);
            std::cout << "[----Dequeue " << id << "] Que.size() = " << que.Size()
            << " head = " << tmp << "\n";
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

    auto waitdequeue = [&](int id) {
        for (int i = 0; i < 5; ++i) {
            int* tmp = new int();
            que.WaitDequeue(tmp);
            std::cout << "[----Dequeue " << id << "] Que.size() = " << que.Size()
            << " head = " << *tmp << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    };

    std::vector<std::thread> threads;

    // // 启动多个读线程
    // for (int i = 0; i < 8; ++i)
    //     threads.emplace_back(enqueue, i);
    
    // // 启动多个读线程
    // for (int i = 0; i < 8; ++i)
    //     threads.emplace_back(waitenque, i);

    // // 启动多个写线程
    // for (int i = 0; i < 5; ++i)
    //     threads.emplace_back(dequeue, i);

    // // 启动多个写线程
    // for (int i = 0; i < 5; ++i)
    //     threads.emplace_back(waitdequeue, i);
    
    for (int i = 0; i < 5; ++i) {
        // threads.emplace_back(waitenque, i);
        // threads.emplace_back(waitdequeue, i);
        threads.emplace_back(enqueue, i);
        // threads.emplace_back(dequeue, i);
    }

    for (auto& t : threads)
        t.join();

    std::cout << "Final que.Size = " << que.Size() << std::endl;
    return 0;
}
