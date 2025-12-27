/**
 * @brief  信号槽机制
 * @date   2025.10.31
 */

#ifndef _RCMW_SIGNAL_H_
#define _RCMW_SIGNAL_H_

#include <mutex>
#include <list>
#include <functional>
#include <memory>
#include <algorithm>

namespace hnu  {
namespace rcmw {
namespace base {

/* 类声明 */
template <typename ... Args>
class Slot;

template <typename ... Args>
class Connection;

/**
 * @brief 信号类：维护了一个槽链表，包含着所有和该信号相连的槽
 */
template <typename ... Args>
class Signal {
public:
    using Callback       = std::function<void(Args...)>;    // 储存回调函数
    using SlotPtr        = std::shared_ptr<Slot<Args...>>;  // 槽指针
    using SlotList       = std::list<SlotPtr>;              // 槽链表：记录该Signal下的所有槽
    using ConnectionType = Connection<Args...>;             // 关联关系

    Signal() {};
    virtual ~Signal() { DisconnectAllSlots(); }

    /* "=" 重载，调用时触发所有已连接的槽 */
    void operator()(Args... args) {
        SlotList local;
        {   // 取 slot
            std::lock_guard<std::mutex> lock(mutex_);
            for(auto& slot : slots_) {
                local.emplace_back(slot);
            }
        }
        if(!local.empty()) {
            for (auto& slot : local) {
                (*slot)(args...);
            }
        }
        ClearDisconnectedSlots();
    }

    /* 连接 slot */
    ConnectionType Connect(const Callback& cb) {
        auto slot = std::make_shared<Slot<Args...>>(cb);
        {   // 存 slot
            std::lock_guard<std::mutex> lock(mutex_);
            slots_.emplace_back(slot);
        }
        return ConnectionType(slot, this);
    }

    /* 断开指定 commection 的 slot */
    bool DisConnect(const ConnectionType& conn) {
        bool find = false;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for(auto& slot : slots_) {
                if(conn.HasSlot(slot)) {
                    find = true;
                    slot->Disconnect();
                }
            }
        }
        if(find) {
            ClearDisconnectedSlots();
        }
        return find;
    }

    /* 断开所有 slot */
    void DisconnectAllSlots() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& slot : slots_) {
            slot->Disconnect();
        }
        slots_.clear();
    }


private:
    Signal(const Signal& other) = delete;
    Signal operator= (const Signal& other) = delete;

    /* 从 slot list 中清理掉断开连接的 slot */
    void ClearDisconnectedSlots() {
        std::lock_guard<std::mutex> lock(mutex_);
        slots_.erase(
            std::remove_if(slots_.begin(), slots_.end(), 
            [](const SlotPtr& slot) { return !slot->connected(); }),
            slots_.end());
    }

    SlotList slots_;
    std::mutex mutex_;
};


/**
 * @brief 槽类：维护了槽的执行函数和连接状态，槽的实例化可以直接传入执行函数
 */
template <typename ... Args>
class Slot {
public:
    using Callback = std::function<void(Args...)>;
    Slot(const Slot& other) : cb_(other.cb_), connected_(other.connected_) {}
    explicit Slot(const Callback& cb, bool connected = true) 
        : cb_(cb), connected_(connected) {}
    virtual ~Slot() {}

    void operator()(Args... args) {
        if(connected_ && cb_) {
            cb_(args...);
        }
    }

    void Disconnect() { connected_ = false; }
    bool connected() const { return connected_; }

private:
    Callback cb_;
    bool connected_ = true;
};


/**
 * @brief 连接类：维护了槽和对应信号的指针，描述槽与信号的连接关系
 */
template <typename ... Args>
class Connection {
public:
    using SlotPtr = std::shared_ptr<Slot<Args...>>;
    using SignalPtr = Signal<Args...>*;

    Connection() :slot_(nullptr), signal_(nullptr) {}
    Connection(const SlotPtr& slot, const SignalPtr& signal)
                :slot_(slot), signal_(signal) {}
    virtual ~Connection() {
        slot_ = nullptr;
        signal_ = nullptr;
    }

    /*  */
    Connection& operator=(const Connection& other) {
        if(this != &other) {
            this->signal_ = other.signal_;
            this->slot_ = other.slot_;
        }
        return *this;
    }

    /* 判断输入的 slot 与 维护的 slot_ 是否为同一个 */
    bool HasSlot(const SlotPtr& slot) const {
        if(slot != nullptr && slot_ != nullptr) {
            return slot_.get() == slot.get();
        }
        return false;
    }

    /* 判断维护的 slot_ 是否连接到信号 */
    bool IsConnected() const {
        if(slot_) {
            return slot_->connected();
        }
        return false;
    }

    /* 断开维护的 slot_ 与 signal_ 的连接 */
    bool Disconnect() {
        if(signal_ && slot_) {
            return signal_ ->DisConnect(*this);
        }
        return false;
    }

private:
    SlotPtr slot_;
    SignalPtr signal_;
};


} // base
} // rcmw
} // hnu 

#endif
