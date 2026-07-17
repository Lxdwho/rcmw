# RCMW - Robot Communication Middleware

高性能机器人通信中间件，基于 CyberRT 设计理念重新实现。

## 环境配置

### 系统要求

- Ubuntu 20.04 / 22.04
- GCC 9+（支持 C++14）
- CMake 3.10+

### 安装步骤

```bash
# 1. 运行环境配置脚本
./setup_env.sh

# 2. 设置环境变量
source ev.sh
```

### 环境变量

| 变量 | 说明 |
|------|------|
| `RCMW_PATH` | 项目根目录 |
| `FASTDDS_PREFIX` | FastDDS 安装路径 |
| `RCMW_IP` | 网络绑定地址（默认 127.0.0.1） |

## 快速开始

### 消息定义

```cpp
#include "serialize/serializable.h"

struct HelloMsg : public Serializable {
    uint64_t seq;
    std::string content;
    SERIALIZE(seq, content)
};
```

### 发布者

```cpp
#include "init.h"
#include "node/node.h"

int main() {
    hnu::rcmw::Init("pub_app");

    auto node = hnu::rcmw::CreateNode("publisher");
    auto pub = node->CreatePublisher<HelloMsg>("hello_topic");

    // 等待订阅者发现
    while (!pub->HasSubscriber()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // 发送消息
    auto msg = std::make_shared<HelloMsg>();
    msg->seq = 1;
    msg->content = "Hello World";
    pub->Publish(msg);

    return 0;
}
```

### 订阅者

```cpp
#include "init.h"
#include "node/node.h"

int main() {
    hnu::rcmw::Init("sub_app");

    auto node = hnu::rcmw::CreateNode("subscriber");
    auto sub = node->CreateSubscriber<HelloMsg>("hello_topic",
        [](const std::shared_ptr<HelloMsg>& msg) {
            std::cout << "Received: " << msg->content << std::endl;
        });

    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;
}
```

## 核心概念

### Node

Node 是用户接口，用于创建 Publisher 和 Subscriber：

```cpp
auto node = hnu::rcmw::CreateNode("node_name", "namespace");
```

### Publisher

发布者用于发送消息：

```cpp
auto pub = node->CreatePublisher<MessageType>("channel_name");
pub->Publish(msg);
```

### Subscriber

订阅者用于接收消息，支持回调模式：

```cpp
auto sub = node->CreateSubscriber<MessageType>("channel_name",
    [](const std::shared_ptr<MessageType>& msg) {
        // 处理消息
    });
```

### Channel

Channel 是通信通道的标识，同一 Channel 的 Pub/Sub 自动匹配：

```cpp
// 不同 Channel 独立通信
auto pub1 = node->CreatePublisher<MsgA>("channel_a");
auto pub2 = node->CreatePublisher<MsgB>("channel_b");
```

## 传输模式

| 模式 | 说明 | 适用场景 |
|------|------|---------|
| SHM | 共享内存传输 | 进程内/进程间高性能通信 |
| RTPS | FastDDS 传输 | 跨网络分布式通信 |

## 性能指标

| 指标 | 数值 |
|------|------|
| 进程内延迟 | ~64 us |
| 最小延迟 | ~8 us |
| P99 延迟 | ~130 us |
| 进程内吞吐量 | >100,000 msg/s |

## 编译

```bash
# 使用 Makefile（测试目录）
cd m_test
make test_main
./test_main.out

# 手动编译
g++ -std=c++14 -I$RCMW_PATH main.cpp -o main \
    -L$FASTDDS_PREFIX/lib -lfastrtps -lfastcdr
```

## 测试用例

```bash
cd m_test

# 查看所有测试
make list

# 运行测试
make hello_pub && ./hello_pub.out    # 发布者
make hello_sub && ./hello_sub.out    # 订阅者
make test_main && ./test_main.out    # 综合测试
make test_advanced && ./test_advanced.out  # 高级场景
```

## 项目结构

```
rcmw/
├── base/           # 基础组件
├── common/         # 公共工具
├── config/         # 配置解析
├── croutine/       # 协程实现
├── discovery/      # 节点发现
├── node/           # 节点、发布者、订阅者
├── scheduler/      # 协程调度器
├── serialize/      # 序列化框架
├── transport/      # 传输层
├── init.h          # 初始化入口
├── state.h         # 状态管理
└── m_test/         # 测试用例
```

## API 参考

### 初始化

```cpp
bool hnu::rcmw::Init(const char* binary_name);
```

### 创建节点

```cpp
std::unique_ptr<Node> hnu::rcmw::CreateNode(
    const std::string& node_name,
    const std::string& name_space = ""
);
```

### 创建发布者

```cpp
template <typename MessageT>
auto Node::CreatePublisher(const std::string& channel_name)
    -> std::shared_ptr<Publisher<MessageT>>;
```

### 创建订阅者

```cpp
template <typename MessageT>
auto Node::CreateSubscriber(
    const std::string& channel_name,
    const CallbackFunc<MessageT>& callback
) -> std::shared_ptr<Subscriber<MessageT>>;
```

### 发布消息

```cpp
bool Publisher::Publish(const std::shared_ptr<MessageT>& msg);
```

## 许可证

MIT License
