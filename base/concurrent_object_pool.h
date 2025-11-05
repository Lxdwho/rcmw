/**
 * @brief 并发对象池
 * @date  2025.11.05
 */

#ifndef _CONCURRENT_OBJECT_POOL_H_
#define _CONCURRENT_OBJECT_POOL_H_

#include <atomic>
#include <memory>

#include "macros.h"
#include "for_each.h"

namespace hnu  {
namespace rcmw {
namespace base {

template<typename T>
class CCObjectPool : public std::enable_shared_from_this<CCObjectPool<T>> {
public:
    explicit CCObjectPool(uint32 size);
    virtual ~CCObjectPool();

    template<typename... Args>
    void ConstructAll(Args&&... args);

    template<typename... Args>
    std::shared_ptr<T> ConstructObject(Args&&... args);

    std::shared_ptr<T> GetObject();
    void ReleaseObject(T*);
    uint32_t size() const;

private:


}




} // base
} // rcmw
} // hnu

#endif
