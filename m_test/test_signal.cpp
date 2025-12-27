#include <iostream>
#include "../base/signal_slot.h"

using namespace hnu::rcmw::base;

void test_signal1(const std::string&& str) {
    std::cout << "test_signal1: " << str << std::endl;
}

void test_signal2(const std::string&& str) {
    std::cout << "test_signal2: " << str << std::endl;
}

void test_signal3(const std::string&& str) {
    std::cout << "test_signal3: " << str << std::endl;
}

void test_signal4(const std::string&& str) {
    std::cout << "test_signal4: " << str << std::endl;
}

int main()
{
    Signal<std::string> sig;
    Connection<std::string> conn1 = sig.Connect(test_signal1);
    Connection<std::string> conn2 = sig.Connect(test_signal2);
    Connection<std::string> conn3 = sig.Connect(test_signal3);
    Connection<std::string> conn4 = sig.Connect(test_signal4);
    Connection<std::string> conn5 = sig.Connect(
        [](const std::string&& str){
        std::cout << "test_lambda: " << str << std::endl;
    });

    sig.DisConnect(conn3);

    sig("Hello");
}
