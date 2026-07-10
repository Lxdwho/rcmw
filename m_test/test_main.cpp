/**
 * @brief  中间件综合测试
 *
 * 测试项目：
 *   1. 基础功能 - 单 Pub 单 Sub 收发
 *   2. 多 Channel - 多个 Topic 同时收发
 *   3. 延迟测试 - 统计消息延迟
 *   4. 吞吐量测试 - 统计 msg/s
 *   5. 压力测试 - 高频发送
 *
 * 编译：make test_main
 * 运行：./test_main.out
 */

#include "../init.h"
#include "../node/node.h"
#include "../serialize/serializable.h"

#include <iostream>
#include <iomanip>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <numeric>
#include <algorithm>
#include <cmath>

using namespace hnu::rcmw;
using namespace hnu::rcmw::logger;

// ──────────────────────────────────────────────────────────────
// 消息结构
// ──────────────────────────────────────────────────────────────
struct TestMsg : public Serializable {
    uint64_t seq;
    uint64_t timestamp_ns;
    uint64_t data;
    SERIALIZE(seq, timestamp_ns, data)
};

// ──────────────────────────────────────────────────────────────
// 工具函数
// ──────────────────────────────────────────────────────────────
static uint64_t NowNs() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

static void PrintHeader(const std::string& title) {
    std::cout << "\n========================================================" << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << "========================================================" << std::endl;
}

static void PrintResult(const std::string& name, int count, double value, const std::string& unit) {
    std::cout << std::left << std::setw(30) << name
              << std::right
              << std::setw(10) << count << " 次"
              << std::setw(12) << std::fixed << std::setprecision(1) << value << " " << unit
              << std::endl;
}

// ──────────────────────────────────────────────────────────────
// 测试 1：基础功能 - 单 Pub 单 Sub
// ──────────────────────────────────────────────────────────────
void TestBasic(std::unique_ptr<Node>& node) {
    PrintHeader("测试 1：基础功能 - 单 Pub 单 Sub");

    const int MSG_COUNT = 100;
    std::atomic<int> received{0};
    std::atomic<bool> first_received{false};

    // 创建 Pub/Sub
    auto pub = node->CreatePublisher<TestMsg>("test_basic");
    auto sub = node->CreateSubscriber<TestMsg>("test_basic",
        [&](const std::shared_ptr<TestMsg>& msg) {
            int count = received.fetch_add(1) + 1;
            if (count == 1) {
                first_received = true;
                std::cout << "  收到第一条消息, seq=" << msg->seq << std::endl;
            }
        });

    // 等待发现
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 发送消息
    for (int i = 0; i < MSG_COUNT; i++) {
        auto msg = std::make_shared<TestMsg>();
        msg->seq = i;
        msg->timestamp_ns = NowNs();
        msg->data = i * 100;
        pub->Publish(msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    // 等待接收完成
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "  发送: " << MSG_COUNT << " 条" << std::endl;
    std::cout << "  接收: " << received.load() << " 条" << std::endl;
    std::cout << "  结果: " << (received.load() == MSG_COUNT ? "✓ 通过" : "✗ 失败") << std::endl;
}

// ──────────────────────────────────────────────────────────────
// 测试 2：多 Channel
// ──────────────────────────────────────────────────────────────
void TestMultiChannel(std::unique_ptr<Node>& node) {
    PrintHeader("测试 2：多 Channel - 3 个 Topic");

    const int CHANNEL_COUNT = 3;
    const int MSG_PER_CHANNEL = 500;
    std::atomic<int> received[CHANNEL_COUNT];
    for (auto& r : received) r = 0;

    std::vector<std::shared_ptr<Publisher<TestMsg>>> pubs;
    std::vector<std::shared_ptr<Subscriber<TestMsg>>> subs;

    // 创建多个 Channel 的 Pub/Sub
    for (int i = 0; i < CHANNEL_COUNT; i++) {
        std::string channel = "test_channel_" + std::to_string(i);
        pubs.push_back(node->CreatePublisher<TestMsg>(channel));
        subs.push_back(node->CreateSubscriber<TestMsg>(channel,
            [&, i](const std::shared_ptr<TestMsg>& msg) {
                received[i]++;
            }));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 每个 Channel 发送消息
    for (int ch = 0; ch < CHANNEL_COUNT; ch++) {
        for (int i = 0; i < MSG_PER_CHANNEL; i++) {
            auto msg = std::make_shared<TestMsg>();
            msg->seq = i;
            msg->timestamp_ns = NowNs();
            msg->data = ch * 1000 + i;
            pubs[ch]->Publish(msg);
            usleep(100);
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "  每个 Channel 发送: " << MSG_PER_CHANNEL << " 条" << std::endl;
    bool all_pass = true;
    for (int i = 0; i < CHANNEL_COUNT; i++) {
        std::cout << "  Channel " << i << " 接收: " << received[i].load() << " 条" << std::endl;
        if (received[i].load() != MSG_PER_CHANNEL) all_pass = false;
    }
    std::cout << "  结果: " << (all_pass ? "✓ 通过" : "✗ 失败") << std::endl;
}

// ──────────────────────────────────────────────────────────────
// 测试 3：延迟测试
// ──────────────────────────────────────────────────────────────
void TestLatency(std::unique_ptr<Node>& node) {
    PrintHeader("测试 3：延迟测试");

    const int MSG_COUNT = 1000;
    std::vector<double> latencies_us;
    std::mutex mtx;

    auto pub = node->CreatePublisher<TestMsg>("test_latency");
    auto sub = node->CreateSubscriber<TestMsg>("test_latency",
        [&](const std::shared_ptr<TestMsg>& msg) {
            double lat = (NowNs() - msg->timestamp_ns) / 1000.0;
            std::lock_guard<std::mutex> lk(mtx);
            latencies_us.push_back(lat);
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 发送消息
    for (int i = 0; i < MSG_COUNT; i++) {
        auto msg = std::make_shared<TestMsg>();
        msg->seq = i;
        msg->timestamp_ns = NowNs();
        msg->data = i;
        pub->Publish(msg);
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (latencies_us.empty()) {
        std::cout << "  未收到任何消息" << std::endl;
        return;
    }

    // 统计延迟
    std::sort(latencies_us.begin(), latencies_us.end());
    double sum = std::accumulate(latencies_us.begin(), latencies_us.end(), 0.0);
    double avg = sum / latencies_us.size();

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "  收到: " << latencies_us.size() << " 条" << std::endl;
    std::cout << "  平均延迟: " << avg << " us" << std::endl;
    std::cout << "  最小延迟: " << latencies_us.front() << " us" << std::endl;
    std::cout << "  最大延迟: " << latencies_us.back() << " us" << std::endl;
    std::cout << "  P50 延迟: " << latencies_us[latencies_us.size() / 2] << " us" << std::endl;
    std::cout << "  P99 延迟: " << latencies_us[(size_t)(latencies_us.size() * 0.99)] << " us" << std::endl;
}

// ──────────────────────────────────────────────────────────────
// 测试 4：吞吐量测试
// ──────────────────────────────────────────────────────────────
void TestThroughput(std::unique_ptr<Node>& node) {
    PrintHeader("测试 4：吞吐量测试");

    const int DURATION_SEC = 3;
    std::atomic<uint64_t> received{0};
    std::atomic<bool> stop{false};

    auto pub = node->CreatePublisher<TestMsg>("test_throughput");
    auto sub = node->CreateSubscriber<TestMsg>("test_throughput",
        [&](const std::shared_ptr<TestMsg>& msg) {
            received++;
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 发送线程
    std::atomic<uint64_t> sent{0};
    std::thread pub_thread([&]{
        uint64_t seq = 0;
        auto start = std::chrono::steady_clock::now();
        while (!stop.load()) {
            auto msg = std::make_shared<TestMsg>();
            msg->seq = seq++;
            msg->timestamp_ns = NowNs();
            msg->data = seq;
            pub->Publish(msg);
            sent++;

            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - start).count();
            if (elapsed >= DURATION_SEC) break;
            usleep(200);
        }
    });

    // 等待测试时间
    std::this_thread::sleep_for(std::chrono::seconds(DURATION_SEC + 1));
    stop = true;
    pub_thread.join();

    uint64_t sent_count = sent.load();
    uint64_t recv_count = received.load();

    std::cout << std::fixed << std::setprecision(0);
    std::cout << "  发送: " << sent_count << " 条 (" << sent_count / DURATION_SEC << " msg/s)" << std::endl;
    std::cout << "  接收: " << recv_count << " 条 (" << recv_count / DURATION_SEC << " msg/s)" << std::endl;
    if (sent_count > 0) {
        std::cout << "  丢包率: " << std::fixed << std::setprecision(2)
                  << (sent_count - recv_count) * 100.0 / sent_count << "%" << std::endl;
    }
}

// ──────────────────────────────────────────────────────────────
// 测试 5：压力测试 - 高频发送
// ──────────────────────────────────────────────────────────────
void TestStress(std::unique_ptr<Node>& node) {
    PrintHeader("测试 5：压力测试 - 高频发送");

    const int MSG_COUNT = 5000;
    std::atomic<uint64_t> received{0};

    auto pub = node->CreatePublisher<TestMsg>("test_stress");
    auto sub = node->CreateSubscriber<TestMsg>("test_stress",
        [&](const std::shared_ptr<TestMsg>& msg) {
            received++;
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 高频发送
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < MSG_COUNT; i++) {
        auto msg = std::make_shared<TestMsg>();
        msg->seq = i;
        msg->timestamp_ns = NowNs();
        msg->data = i;
        pub->Publish(msg);
        usleep(100);
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    uint64_t recv_count = received.load();
    double throughput = recv_count * 1000.0 / elapsed;

    std::cout << std::fixed << std::setprecision(0);
    std::cout << "  发送: " << MSG_COUNT << " 条" << std::endl;
    std::cout << "  接收: " << recv_count << " 条" << std::endl;
    std::cout << "  耗时: " << elapsed << " ms" << std::endl;
    std::cout << "  吞吐量: " << throughput << " msg/s" << std::endl;
    std::cout << "  结果: " << (recv_count == MSG_COUNT ? "✓ 通过" : "✗ 失败") << std::endl;
}

// ──────────────────────────────────────────────────────────────
// 主函数
// ──────────────────────────────────────────────────────────────
int main() {
    hnu::rcmw::Init("test_main");
    Logger::Get_instance()->level(Logger::LOG_INFO);

    std::cout << "============================================================" << std::endl;
    std::cout << "  中间件综合测试" << std::endl;
    std::cout << "============================================================" << std::endl;

    // 创建 Node
    auto node = CreateNode("test_node", "rcmw");

    // 运行测试
    TestBasic(node);
    TestMultiChannel(node);
    TestLatency(node);
    TestThroughput(node);
    TestStress(node);

    std::cout << "\n============================================================" << std::endl;
    std::cout << "  所有测试完成" << std::endl;
    std::cout << "============================================================" << std::endl;

    return 0;
}
