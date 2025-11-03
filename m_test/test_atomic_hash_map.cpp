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
