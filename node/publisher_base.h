/**
 * @brief 发布者基类
 * @date 2026.01.05
 */

#ifndef _RCMW_NODE_PUBLISHERBASE_H_
#define _RCMW_NODE_PUBLISHERBASE_H_

#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#include "rcmw/config/RoleAttributes.h"

namespace hnu   {
namespace rcmw  {

using namespace config;

/**
 * @brief 发布者基类
 */
class PublisherBase {
public:
    explicit PublisherBase(const RoleAttributes& role_attr) 
            : role_attr_(role_attr), init_(false) {}
    virtual ~PublisherBase() {}

    virtual bool Init() = 0;
    virtual void Shutdown() = 0;

    virtual bool HasSubscriber() { return false; }
    virtual void GetSubscribers(std::vector<RoleAttributes>* subscribers) {}

    const std::string& GetChannelName() const {
        return role_attr_.channel_name;
    }

    bool IsInit() const {
        std::lock_guard<std::mutex> lg(lock_);
        return init_;
    }
protected:
    RoleAttributes role_attr_;
    std::atomic<bool> init_;
    mutable std::mutex lock_;
};

} // rcmw
} // hnu

#endif
