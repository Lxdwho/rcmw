/**
 * @brief  Hello World - 订阅者
 *
 * 编译：make hello_sub
 * 运行：./hello_sub.out
 */

#include "../init.h"
#include "../node/node.h"
#include "../serialize/serializable.h"

#include <iostream>
#include <chrono>
#include <thread>

using namespace hnu::rcmw;

// 消息结构（必须和发布者一致）
struct HelloMsg : public Serializable {
    uint64_t seq;
    std::string content;
    SERIALIZE(seq, content)
};

int main() {
    hnu::rcmw::Init("hello_sub");
    Logger::Get_instance()->level(Logger::LOG_INFO);

    // 1. 创建节点
    auto node = CreateNode("hello_subscriber");

    // 2. 创建订阅者（注册回调）
    auto sub = node->CreateSubscriber<HelloMsg>("hello_topic",
        [](const std::shared_ptr<HelloMsg>& msg) {
            std::cout << "[Sub] 收到: " << msg->content << std::endl;
        });

    // 3. 等待接收消息
    std::cout << "[Sub] 等待消息..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(15));

    std::cout << "[Sub] 结束" << std::endl;
    return 0;
}
