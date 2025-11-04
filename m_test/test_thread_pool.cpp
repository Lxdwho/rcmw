#include <iostream>
#include <vector>
#include <thread>
#include <unistd.h>
#include "../base/thread_pool.h"

using namespace hnu::rcmw::base;

int main() {
    ThreadPool thr = ThreadPool(2, 8);
    auto fur1 = thr.Enqueue([](std::string str) {
        std::cout << str << std::endl;
    }, "YES: lambad1");
    auto fur2 = thr.Enqueue([](std::string str) {
        std::cout << str << std::endl;
    }, "YES: lambad2");
    auto fur3 = thr.Enqueue([](std::string str) {
        std::cout << str << std::endl;
    }, "YES: lambad3");
    auto fur4 = thr.Enqueue([](std::string str) {
        std::cout << str << std::endl;
    }, "YES: lambad4");
    fur1.get(), fur2.get(), fur3.get(), fur4.get();
    return 0;
}
