/**
 * @brief 基于shm的rceiver
 * @date 2025.12.10
 */

#ifndef _TRANSPORT_SHM_TRANSPORT_H_
#define _TRANSPORT_SHM_TRANSPORT_H_

#include "receiver.h"
#include "rcmw/logger/log.h"
#include "rcmw/transport/dispatcher/shm_dispatcher.h"
#include <functional>

namespace hnu       {
namespace rcmw      {
namespace transport {

template<typename M>
class ShmReceiver : public Receiver<M> {
public:
    ShmReceiver(const RoleAttributes& attr, 
                const typename Receiver<M>::MessageListener& msg_listener);
    virtual ~ShmReceiver();

    void Enable() override;
    void Disable() override;

    void Enable(const RoleAttributes& opposite_attr) override;
    void Disable(const RoleAttributes& opposite_attr) override;
private:
    ShmDispatcherPtr dispatcher_;
};

/**
 * @brief 构造函数
 * @param attr: 角色属性
 * @param msg_listener: 回调函数包装器
 */
template<typename M>
ShmReceiver<M>::ShmReceiver(const RoleAttributes& attr,
            const typename Receiver<M>::MessageListener& msg_listener) :
            Receiver<M>(attr, msg_listener) {
    dispatcher_ = ShmDispatcher::Instance();
}

template<typename M>
ShmReceiver<M>::~ShmReceiver() { Disable(); }

/**
 * @brief 添加Listener
 */
template<typename M>
void ShmReceiver<M>::Enable() {
    if(this->enabled_) return;
    dispatcher_->AddListener<M>(this->attr_, 
                    std::bind(&ShmReceiver<M>::OnNewMessage, this,
                    std::placeholders::_1, std::placeholders::_2));
    this->enabled_ = true;
}

/**
 * @brief 删除Listener
 */
template<typename M>
void ShmReceiver<M>::Disable() {
    if(!this->enabled_) return;
    dispatcher_->RemoveListener<M>(this->attr_);
    this->enabled_ = false;
}

/**
 * @brief 添加Listener，带opposite_attr
 */
template<typename M>
void ShmReceiver<M>::Enable(const RoleAttributes& opposite_attr) {
    dispatcher_->AddListener<M>(this->attr_, opposite_attr, 
        std::bind(&ShmReceiver<M>::OnNewMessage, this,
        std::placeholders::_1, std::placeholders::_2));
}

/**
 * @brief 删除Listener，带opposite_attr
 */
template<typename M>
void ShmReceiver<M>::Disable(const RoleAttributes& opposite_attr) {
    dispatcher_->RemoveListener<M>(this->attr_, opposite_attr);
}

} // transport
} // rcmw
} // hnu

#endif
