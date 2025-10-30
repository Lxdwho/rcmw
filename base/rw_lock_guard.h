/**
 * @brief  读写锁守卫
 * @date   2025.10.30
 * @author lxd
 */


#include <mutex>

namespace hnu   {  
namespace rcmw  {
namespace base  {

/**
 * @brief 读锁守卫，创建时自动加锁，销毁时自动解锁
 */
template <typename RWLock>
class ReadLockGuard {
public:
    explicit ReadLockGuard(RWLock& lock) : rw_lock_(lock) { rw_lock_.ReadLock(); }
    ~ReadLockGuard() { rw_lock_.ReadUnlock(); }

private:
    ReadLockGuard(const ReadLockGuard& other) = delete;
    ReadLockGuard& operator=(const  ReadLockGuard& other) = delete;
    RWLock& rw_lock_;
};

/**
 * @brief 写锁守卫，创建时自动加锁，销毁时自动解锁
 */
template<typename RWLock>
class WriteLockGuard {
public:
    explicit WriteLockGuard(RWLock& lock) : rw_lock_(lock) {
        rw_lock_.WriteLock();
    }
    ~WriteLockGuard() { rw_lock_.WriteUnlock(); }

private:
    WriteLockGuard(const WriteLockGuard& other) = delete;
    WriteLockGuard& operator=(const WriteLockGuard& other) = delete;
    RWLock& rw_lock_;
};


} // namespace hnu 
} // namespace rcmw
} // namespace base
