# 对象池详解
>对象池是一种设计模式，以空间换时间的方式，保证程序运行的效率。它首先申请一块内存，并在其中实例化`n`个对象，当需要使用时调用相关的接口进行取对象操作，使用完毕后进行归还。

cmw中的对象池声明如下：
```cpp
template <typename T>
class ObjectPool : public std::enable_shared_from_this<ObjectPool<T>> {
public:
    using InitFunc = std::function<void(T *)>;
    using ObjectPoolPtr = std::shared_ptr<ObjectPool<T>>;

    template <typename... Args>
    explicit ObjectPool(uint32_t num_objects, Args &&... args);

    template <typename... Args>
    ObjectPool(uint32_t num_objects, InitFunc f, Args &&... args);

    virtual ~ObjectPool();

    /* 拿到一个对象*/
    std::shared_ptr<T> GetObject();

private:
    struct Node {
        T object;
        Node *next;
    };
    /*禁用拷贝构造*/
    ObjectPool(ObjectPool &) = delete;
    ObjectPool &operator=(ObjectPool &) = delete;
    void ReleaseObject(T *);

    uint32_t num_objects_ = 0;
    char *object_arena_ = nullptr;
    Node *free_head_ = nullptr;
};
```
如上对象池是一个模板类，模板对象是需要存在池中的对象类型。对象池类中维护了3个变量:`num_objects_`, `object_arena_`, `free_head_`。分别为对象池的大小、对象池申请空间的首地址以及对象链表的表头。同时内部存在一个结构体`Node`，因为对象池中的对象是以单向链表的形式进行访问的。

此外，类内提供了以下函数用于对外提供服务：
```cpp
    /* 池构造函数 */
    template <typename... Args>
    explicit ObjectPool(uint32_t num_objects, Args &&... args);
    /* 池构造函数 */
    template <typename... Args>
    ObjectPool(uint32_t num_objects, InitFunc f, Args &&... args);
    /* 池析构函数 */
    virtual ~ObjectPool();
    /* 拿到对象*/
    std::shared_ptr<T> GetObject();
    /* 释放对象 */
    void ReleaseObject(T *);
```
## 1 池的构造
对象池类中包含两个构造函数，第一个构造函数如下：
```cpp
template <typename T>
template <typename... Args>
ObjectPool<T>::ObjectPool(uint32_t num_objects, Args &&... args)
    : num_objects_(num_objects) {
    const size_t size = sizeof(Node);
    object_arena_ = static_cast<char *>(std::calloc(num_objects_, size));
    if (object_arena_ == nullptr) {
        throw std::bad_alloc();
    }

    FOR_EACH(i, 0, num_objects_) {
        T *obj = new (object_arena_ + i * size) T(std::forward<Args> (args)...);
        reinterpret_cast<Node *>(obj)->next = free_head_;
        free_head_ = reinterpret_cast<Node *>(obj);
    }
}
```
如上，池的构造函数也是一个模板函数，接收一个参数列表用于构造池中的对象。该构造函数接受两个参数一个是对象池的大小`num_objects`，它直接赋值给`num_objects_`用于指定对象池大小；，第二个即为参数列表，用于池中对象构造时传入参数
在指定池大小后使用`calloc`申请对应大小的空间如下：
```cpp
const size_t size = sizeof(Node);
object_arena_ = static_cast<char *>(std::calloc(num_objects_, size));
if (object_arena_ == nullptr) {
    throw std::bad_alloc();
}
```
空间的大小与对象池大小对应，申请完成后转为`char*`类型首地址保存在`object_arena_`中。随后使用`FOR_EACH`进行遍历,对每个对象进行空间的申请以及初始化
```cpp
FOR_EACH(i, 0, num_objects_) {
    T *obj = new (object_arena_ + i * size) T(std::forward<Args> (args)...);
    reinterpret_cast<Node *>(obj)->next = free_head_;
    free_head_ = reinterpret_cast<Node *>(obj);
}
```
首先使用`new () T()`在指定的地址上去初始化对象T，随后使用`reinterpret_cast`将`obj`指针转为`Node*`将类型，然后进行链表的连接操作。
>这里`reinterpret_cast`之所以可以将T类型的指针转为Node类型，应该是因为T类型是Node类型中的第一个成员，并且T类型符合标准分布？

第二个构造函数在第一个的基础上多了一个传入参数，用于对`obj`对象进行初始化
```cpp
template <typename T>
template <typename... Args>
ObjectPool<T>::ObjectPool(uint32_t num_objects, InitFunc f, Args &&... args)
    : num_objects_(num_objects) {
    const size_t size = sizeof(Node);
    object_arena_ = static_cast<char *>(std::calloc(num_objects_, size));
    if (object_arena_ == nullptr) {
        throw std::bad_alloc();
    }

    FOR_EACH(i, 0, num_objects_) {
        T *obj = new (object_arena_ + i * size) T(std::forward<Args>(args)...);
        f(obj);
        reinterpret_cast<Node *>(obj)->next = free_head_;
        free_head_ = reinterpret_cast<Node *>(obj);
    }
}
```
## 2 对象的获取
对象的获取通过`std::shared_ptr<T> GetObject();`进行实现，该函数的定义如下：
```cpp
template <typename T>
std::shared_ptr<T> ObjectPool<T>::GetObject() {
    if (cyber_unlikely(free_head_ == nullptr)) {
        return nullptr;
    }

    auto self = this->shared_from_this();
    auto obj = std::shared_ptr<T>(reinterpret_cast<T *>(free_head_),
                            [self](T *object) { self->ReleaseObject(object); });
    free_head_ = free_head_->next;
    return obj;
}
```
函数首先判断了链表表头`free_head_`是否为空指针，不为空则获取类自身指针，这里使用到了`shared_from_this`进行获取，随后将表头对象取出使用智能指针进行管理得到`obj`，并使用lambda表达式指定了对象的析构方式由函数`ReleaseObject`进行
## 3 对象的回收
对象的回收也是同样的链表操作，在对象的获取时，我们指定了对象的析构方式，为函数`ReleaseObject`，也就是说外部使用完这个对象时并不会真正的析构他，而是执行`ReleaseObject`在这里面我们只需要将对象重新放回链表中即可完成回收,函数定义如下:
```cpp
template <typename T>
void ObjectPool<T>::ReleaseObject(T *object) {
    if (cyber_unlikely(object == nullptr)) {
        return;
    }
    reinterpret_cast<Node *>(object)->next = free_head_;
    free_head_ = reinterpret_cast<Node *>(object);
}
```

## 4 总结
对象池的具体行为就是在构造时根据池的大小开辟了一块空间,并在这个空间中以链表的形式实例化了n个对象,在每次需要对象的时候取链表的表头给到外部,使用完成后再将对象添加到表头进行回收
这些功能的具体实现使用到了:
calloc
reinterpret_cast
new (address) T(args);等
此外还用到了一些宏函数后续详述