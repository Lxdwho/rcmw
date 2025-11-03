#include <iostream>
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
        process(std::move(arg));
    }
};

using namespace hnu::rcmw::base;

int main()
{
    int a = 10;  // a是左值
    int b = std::move(a); // 右值引用

    std::string as = "Hello";
    std::string bs = std::move(as);

    std::cout << a << " " << b << std::endl;
    std::cout << as << " " << bs << std::endl;


    int c = 20;
    forwardExample(c);
    forwardExample(30);

    
    std::cout << "-----------------------------" << std::endl;
    int d = 20;
    Test<int> test;
    // test.forwardExample(d);
    test.forwardExample(30);
}
