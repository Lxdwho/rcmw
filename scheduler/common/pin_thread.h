/**
 * @brief 
 * @date 2025.12.27
 */

#ifndef _RCMW_SCHEDULER_COMMON_PIN_THREAD_H_
#define _RCMW_SCHEDULER_COMMON_PIN_THREAD_H_

#include <string>
#include <thread>
#include <vector>
#include "rcmw/logger/log.h"

namespace hnu       {
namespace rcmw      {
namespace scheduler {

/**
 * @brief 解析cpuset
 * @param str 原始配置，如0-7,12,16-23
 * @param cpuset 解析后的配置
 */
void ParseCpuset(const std::string& str, std::vector<int>* cpuset);

/**
 * @brief 设置CPU亲和度策略
 * @param thread 线程
 * @param cpus
 * @param affinity 亲和度策略
 * @param cpu_id
 */
void SetSchedAffinity(std::thread* thread, const std::vector<int>& cpus, 
                        const std::string& affinity, int cpu_id = -1);

/**
 * @brief 设置调度策略
 * @param thread 线程
 * @param spolicy 调度策略
 * @param sched_priority ？
 * @param tid 线程id
 */
void SetSchedPolicy(std::thread* thread, std::string spolicy, 
                    int sched_priority, pid_t tid = -1);

} // scheduler
} // rcmw
} // hnu

#endif
