/**
 * @brief 共享内存状态类
 * @date 2025.11.12
 */

#ifndef _STATE_H_
#define _STATE_H_

#include <atomic>
#include <mutex>

namespace hnu       {
namespace rcmw      {
namespace transport {

class State {
public:
    explicit State(const uint64_t& ceiling_msg_size);
    virtual ~State();

    /* 减少引用计数 */
    void DecreaseReferenceCount() {
        uint32_t current_reference_count = reference_count_.load();
        do{
            if(current_reference_count == 0) return;
        }
        while(!reference_count_.compare_exchange_strong(
            current_reference_count, current_reference_count - 1));
    }
    /* 增加引用计数 */
    void IncreaseReferenceCount() { reference_count_.fetch_add(1); }
    
    /* 变量操作 */
    uint32_t FetchAddSeq(uint32_t diff) { return seq_.fetch_add(diff); }
    uint32_t seq() { return seq_.load(); }
    
    void set_need_remap(bool need_remap) { need_remap_.store(need_remap); }
    bool need_remap() { return need_remap_.load(); }
    
    uint64_t ceiling_msg_size() { return ceiling_msg_size_.load(); }
    uint32_t reference_count() { return reference_count_.load(); }

private:
    std::atomic<bool> need_remap_ = {false};            // 
    std::atomic<uint32_t> seq_ = { 0 };                 // 
    std::atomic<uint32_t> reference_count_ = { 0 };     // 引用计数
    std::atomic<uint32_t> ceiling_msg_size_;            // 
};

} // transport
} // rcmw
} // hnu

#endif
