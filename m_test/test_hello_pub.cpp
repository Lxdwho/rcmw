/**
 * @brief  Hello World - 发布者
 *
 * 编译：make hello_pub
 * 运行：./hello_pub.out
 */

#include "../init.h"
#include "../node/node.h"
#include "../serialize/serializable.h"

#include <iostream>
#include <chrono>
#include <thread>

using namespace hnu::rcmw;

// 消息结构
struct HelloMsg : public Serializable {
    uint64_t seq;
    std::string content;
    SERIALIZE(seq, content)
};

int main() {
    hnu::rcmw::Init("hello_pub");
    Logger::Get_instance()->level(Logger::LOG_INFO);

    // 1. 创建节点
    auto node = CreateNode("hello_publisher");

    // 2. 创建发布者
    auto pub = node->CreatePublisher<HelloMsg>("hello_topic");

    // 3. 等待订阅者发现
    std::cout << "[Pub] 等待订阅者..." << std::endl;
    while (!pub->HasSubscriber()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "[Pub] 发现订阅者，开始发送" << std::endl;

    // 4. 发送消息
    for (int i = 1; i <= 10; i++) {
        auto msg = std::make_shared<HelloMsg>();
        msg->seq = i;
        msg->content = "Hello World " + std::to_string(i);

        pub->Publish(msg);
        std::cout << "[Pub] 发送: " << msg->content << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "[Pub] 发送完成" << std::endl;
    return 0;
}
