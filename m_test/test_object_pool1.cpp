#include "../base/object_pool.h"
#include <iostream>

using namespace hnu::rcmw::base;

class A {
private:
    int a;
public:
    int geta() {
        return a;
    }
    void seta(int b) {
        a = b;
    }
    A(int m) : a(m) {}
};

void useit(std::shared_ptr<A> o) {
    std::cout << o->geta() << std::endl;
    o->seta(100);
    std::cout << o->geta() << std::endl;
}

int main() 
{
    auto oa =  std::make_shared<ObjectPool<A>>(5, 0);

    auto ma = oa->GetObject();
    auto mb = oa->GetObject();
    auto mc = oa->GetObject();
    auto md = oa->GetObject();
    auto me = oa->GetObject();
    auto mf = oa->GetObject();
    useit(ma);
    useit(mb);
    useit(mc);
    useit(md);
    useit(me);
    useit(mf);
    return 0;
}
