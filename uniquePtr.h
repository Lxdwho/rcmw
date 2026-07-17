#include <iostream>

template <typename T>
class uniquePtr {
public:

    explicit uniquePtr(T* ptr = nullptr) : ptr_(ptr) {}
    ~uniquePtr() { delete ptr_; }
    uniquePtr(const uniquePtr& otr) = delete;
    uniquePtr(uniquePtr&& otr) noexcept : ptr_(otr.ptr_) {
        otr.ptr_ = nullptr;
    }

    uniquePtr& operator=(const uniquePtr& otr) = delete;
    uniquePtr& operator=(uniquePtr&& otr) noexcept {
        if(this != &otr) {
            delete ptr_;
            ptr_ = otr.ptr_;
            otr.ptr_ = nullptr;
        }
        return *this;
    }

    T* release() {
        T* tmp = ptr_;
        ptr_ = nullptr;
        return tmp;
    }

    void printa();
    T* operator->() const noexcept { return ptr_; }
    T& operator*() const noexcept { return *ptr_; }
    T* get() const noexcept { return ptr_; }

    explicit operator bool() { return ptr_ != nullptr; }

private:
    T* ptr_;
};