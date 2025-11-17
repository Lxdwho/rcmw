/**
 * @brief 
 * @date 2025.11.15
 */

#ifndef _PERF_EVENT_CACHE_H_
#define _PERF_EVENT_CACHE_H_

#include "perf_event.h"
#include "rcmw/base/bounded_queue.h"
#include <string>
#include <thread>
#include <fstream>

namespace hnu   {
namespace rcmw  {
namespace event {

class PerfEventCache {
public:
    using EventBasePtr = std::shared_ptr<EventBase>;
    ~PerfEventCache();
    void AddTransportEvent(const TransPerf event_id, const uint64_t channel_id, 
                        const uint64_t msg_seq, const uint64_t stamp = 0, 
                        const std::string& adder = "-");
    std::string PerfFile() { return perf_file_; }
    void Shutdown();

private:
    void Start();
    void Run();

    std::thread io_thread_;
    std::ofstream of_;

    bool enable_ = false;
    bool shutdown_ = false;

    std::string perf_file_ = "";
    base::BoundedQueue<EventBasePtr> event_queue_;

    const int KFlushSize = 512;
    const uint64_t KEventQueueSize = 8192;
    DECLARE_SINGLETON(PerfEventCache)
};

} // transport
} // rcmw
} // hnu

#endif
