/**
 * @brief 
 * @date 2025.11.15
 */

#include "perf_event_cache.h"
#include "rcmw/logger/log.h"

namespace hnu   {
namespace rcmw  {
namespace event {

PerfEventCache::PerfEventCache() {
    if(enable_) {
        if(!event_queue_.Init(KEventQueueSize)) {
            AERROR << "Event queue init failed.";
            throw std::runtime_error("Event queue init failed.");
        }
        Start();
    }
}

void PerfEventCache::AddTransportEvent(const TransPerf event_id, const uint64_t channel_id, 
                    const uint64_t msg_seq, const uint64_t stamp = 0, 
                    const std::string& adder = "-") {
    if(!enable_) return;
    EventBasePtr e = std::make_shared<TransportEvent>();
    e->set_eid(static_cast<int>(event_id));
    e->set_channel_id(channel_id);
    e->set_msg_seq(msg_seq);
    e->set_adder(adder);
    e->set_stamp(stamp);
    event_queue_.Enqueue(e);
}

PerfEventCache::~PerfEventCache() { Shutdown(); }

void PerfEventCache::Shutdown() {
    if(!enable_) return;
    shutdown_ = true;
    event_queue_.BreakAllWait();
    if(io_thread_.joinable()) io_thread_.join();
    of_.flush();
    of_.close();
}

void PerfEventCache::Start() {
    std::string perf_file = "rcmw_perf_.data";
    std::replace(perf_file.begin(), perf_file.end(), ' ', '_');
    std::replace(perf_file.begin(), perf_file.end(), ':', '-');
    of_.open(perf_file, std::ios::trunc);
    perf_file_ = perf_file;
    io_thread_ = std::thread(&PerfEventCache::Run, this);
}

void PerfEventCache::Run() {
    EventBasePtr event;
    int buf_size = 0;
    while(!shutdown_) {
        if(event_queue_.WaitDequeue(&event)) {
            of_ << event->SerializeTostring() << std::endl;
            buf_size++;
            if(buf_size >= KFlushSize) {
                of_.flush();
                buf_size = 0;
            }
        }
    }
}

} // transport
} // rcmw
} // hnu
