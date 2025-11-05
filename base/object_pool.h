/**
 * @brief 对象池
 * @date  2025.11.05
 */

#ifndef _OBJECT_POOL_H_
#define _OBJECT_POOL_H_

#include <memory>
#include <functional>
#include "macros.h"
#include "for_each.h"

namespace hnu  {
namespace rcmw {
namespace base {

template<typename T>
class ObjectPool : public std::enable_shared_from_this<ObjectPool<T>> {
public:
    using InitFunc = std::function<void(T*)>;
    using ObjectPoolPtr = std::shared_ptr<ObjectPool<T>>;

    template<typename... Args>
    explicit ObjectPool(uint32_t num_objects, Args &&... args);

    template<typename... Args>
    ObjectPool(uint32_t num_objects, InitFunc f, Args &&... args);

    virtual ~ObjectPool();

    std::shared_ptr<T> GetObject();

private:
    struct Node
    {
        T object;
        Node *Next;
    };

    ObjectPool(const ObjectPool& other) = delete;
    ObjectPool& operator= (const ObjectPool& other) = delete;
    void ReleaseObject(T*);

    uint32_t num_objects_ = 0;      // 对象数量
    char *object_arena_ = nullptr;  // 内存起始地址
    Node *free_head_ = nullptr;     // 链表形式存储，保存表头
};

/**
 * @brief  构造函数
 * @tparam T：对象类型
 * @tparam Args：对象构造参数类型
 * @param  num_objects: 对象池大小
 * @param  args:对象构造参数
 */
template<typename T>
template<typename... Args>
ObjectPool<T>::ObjectPool(uint32_t num_objects, Args &&... args) 
                            : num_objects_(num_objects) {
    const size_t size = sizeof(Node);
    object_arena_ = static_cast<char*>(CheckedCalloc(num_objects_, size));
    FOR_EACH(i, 0, num_objects_) {
        T* obj = new(object_arena_ + i * size) T(std::forward<Args>(args)...);
        reinterpret_cast<Node*>(obj)->Next = free_head_;
        free_head_ = reinterpret_cast<Node*>(obj);
    }
}

/**
 * @brief  构造函数
 * @tparam T：对象类型
 * @tparam Args：对象构造参数类型
 * @param  num_objects: 对象池大小
 * @param  args:对象构造参数
 * @param  f：对象的初始化函数？？？？？？？？？？？？
 */
template<typename T>
template<typename... Args>
ObjectPool<T>::ObjectPool(uint32_t num_objects, InitFunc f, Args &&... args) 
                            : num_objects_(num_objects) {
    const size_t size = sizeof(Node);
    object_arena_ = static_cast<char*>(CheckedCalloc(num_objects_, size));
    FOR_EACH(i, 0, num_objects_) {
        T* obj = new(object_arena_ + i * size) T(std::forward<Args>(args)...);
        f(obj);
        reinterpret_cast<Node*>(obj)->Next = free_head_;
        free_head_ = reinterpret_cast<Node*>(obj);
    }
}

/* 析构、free内存 */
template<typename T>
ObjectPool<T>::~ObjectPool() {
    if(object_arena_ != nullptr) {
        const size_t size = sizeof(Node);
        FOR_EACH(i, 0, num_objects_) {
            reinterpret_cast<Node*>(object_arena_ + i * size)->object.~T();
        }
        std::free(object_arena_);
    }
}

/* 取对象 */
template<typename T>
std::shared_ptr<T> ObjectPool<T>::GetObject() {
    if(cyber_unlikely(free_head_ == nullptr)) {
        return nullptr;
    }
    auto self = this->shared_from_this();
    auto obj = std::shared_ptr<T>(reinterpret_cast<T*>(free_head_), 
                [self](T* object) { self->ReleaseObject(object); });
    free_head_ = free_head_->Next;
    return obj;
}

/* 回收对象 */
template<typename T>
void ObjectPool<T>::ReleaseObject(T* object) {
    if(cyber_unlikely(object == nullptr)) {
        return;
    }
    reinterpret_cast<Node*>(object)->Next = free_head_;
    free_head_ = reinterpret_cast<Node*>(object);
}

} // base
} // rcmw
} // hnu

#endif
