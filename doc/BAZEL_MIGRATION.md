# RCMW: Make → Bazel 迁移指南

## 目录

- [背景](#背景)
- [第三方依赖处理](#第三方依赖处理)
- [WORKSPACE](#workspace)
- [.bazelrc](#bazelrc)
- [各模块 BUILD 文件](#各模块-build-文件)
- [测试文件 include 修正](#测试文件-include-修正)
- [m_test/BUILD](#m_testbuild)
- [验证构建](#验证构建)

---

## 背景

当前 Makefile 构建存在以下问题：

- 硬编码第三方库绝对路径（`/home/mi/Fast-DDS/install/...`）
- 需要先 `source ev.sh` 设置环境变量
- 增量编译依赖追踪不够精确

Bazel 可以实现：声明式依赖管理、精确增量构建、可复现构建、无需手动设置环境变量。

### 第三方依赖概览

| 依赖 | 类型 | Bazel 引入方式 |
|------|------|---------------|
| GoogleTest | 静态库 (.a) | `http_archive`（官方支持 Bazel） |
| FastDDS | 动态库 (.so) | `http_archive` + `rules_foreign_cc`（CMake 编译） |
| nlohmann/json | 纯头文件 | `http_archive` |

---

## 第三方依赖处理

项目使用 `http_archive` 从源码编译第三方库，无需 `thirdparty/` 目录中的预编译产物。

- **GoogleTest**：官方提供 Bazel BUILD 文件，直接可用
- **FastDDS**：无官方 Bazel 支持，通过 `rules_foreign_cc` 的 `cmake()` 规则从源码编译
- **nlohmann/json**：纯头文件库，只需声明 `hdrs` 和 `includes`

---

## WORKSPACE

替换项目根目录的 `WORKSPACE` 文件：

```python
workspace(name = "rcmw")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# ============================================================
# rules_foreign_cc — 用于编译 FastDDS (CMake 项目)
# ============================================================
http_archive(
    name = "rules_foreign_cc",
    sha256 = "",  # 下载后 Bazel 会提示正确的 sha256
    strip_prefix = "rules_foreign_cc-0.12.0",
    url = "https://github.com/bazelbuild/rules_foreign_cc/releases/download/0.12.0/rules_foreign_cc-0.12.0.tar.gz",
)

load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")

rules_foreign_cc_dependencies()

# ============================================================
# GoogleTest (官方 Bazel 支持)
# ============================================================
http_archive(
    name = "com_google_googletest",
    sha256 = "",
    strip_prefix = "googletest-1.15.2",
    urls = ["https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz"],
)

# ============================================================
# nlohmann/json (纯头文件)
# ============================================================
http_archive(
    name = "json",
    build_file_content = """
cc_library(
    name = "nlohmann_json",
    hdrs = glob(["include/**/*.hpp"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
""",
    sha256 = "",
    strip_prefix = "json-3.11.3",
    urls = ["https://github.com/nlohmann/json/releases/download/v3.11.3/json-3.11.3.tar.xz"],
)

# ============================================================
# FastCDR (FastDDS 的依赖)
# ============================================================
http_archive(
    name = "fastcdr",
    build_file_content = """
load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

cmake(
    name = "fastcdr",
    lib_source = glob(["**"]),
    visibility = ["//visibility:public"],
)
""",
    sha256 = "",
    strip_prefix = "Fast-CDR-1.1.0",
    urls = ["https://github.com/eProsima/Fast-CDR/archive/refs/tags/v1.1.0.tar.gz"],
)

# ============================================================
# FastDDS
# ============================================================
http_archive(
    name = "fastrtps",
    build_file_content = """
load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

cmake(
    name = "fastrtps",
    lib_source = glob(["**"]),
    deps = ["@fastcdr//:fastcdr"],
    visibility = ["//visibility:public"],
)
""",
    sha256 = "",
    strip_prefix = "Fast-DDS-2.3.0",
    urls = ["https://github.com/eProsima/Fast-DDS/archive/refs/tags/v2.3.0.tar.gz"],
)
```

> **注意**：
> - `sha256` 留空即可，首次构建时 Bazel 会在终端打印正确的值，复制填入即可
> - 版本号请根据 `thirdparty/` 中实际使用的版本调整（FastDDS 2.3.0、GoogleTest 1.15.2 等）
> - FastDDS 编译较慢（约 5-10 分钟），首次构建需要耐心

---

## .bazelrc

在项目根目录创建 `.bazelrc` 文件：

```
# C++ 标准
build --cxxopt=-std=c++14

# 系统链接库
build --linkopt=-lpthread
build --linkopt=-luuid
build --linkopt=-lrt
build --linkopt=-latomic
build --linkopt=-ldl
```

---

## 各模块 BUILD 文件

> **include 路径说明**：项目使用 `#include "xxx"` 风格的头文件引用。
> 每个 `cc_library` 需要设置 `includes = [".."]` 使 Bazel sandbox 中 `rcmw/` 路径可解析。
> 如果遇到头文件找不到的问题，检查 `includes` 配置或使用 `bazel build --sandbox_debug` 调试。

### base/ — 纯头文件

```python
cc_library(
    name = "base",
    hdrs = glob(["*.h"]),
    includes = [".."],
    visibility = ["//visibility:public"],
)
```

### logger/

```python
cc_library(
    name = "logger",
    srcs = ["logger.cpp"],
    hdrs = glob(["*.h"]),
    includes = [".."],
    visibility = ["//visibility:public"],
)
```

### common/

```python
cc_library(
    name = "common",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    includes = [".."],
    deps = [
        "//logger",
        "@json//:nlohmann_json",
    ],
    visibility = ["//visibility:public"],
)
```

### time/

```python
cc_library(
    name = "time",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    includes = [".."],
    deps = ["//common"],
    visibility = ["//visibility:public"],
)
```

### serialize/

```python
cc_library(
    name = "serialize",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    includes = [".."],
    visibility = ["//visibility:public"],
)
```

### config/

```python
cc_library(
    name = "config",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    includes = [".."],
    deps = [
        "//common",
        "//logger",
        "@json//:nlohmann_json",
    ],
    visibility = ["//visibility:public"],
)
```

### event/

```python
cc_library(
    name = "event",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    includes = [".."],
    deps = ["//common", "//logger"],
    visibility = ["//visibility:public"],
)
```

### blocker/

```python
cc_library(
    name = "blocker",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    includes = [".."],
    deps = ["//base", "//common", "//config"],
    visibility = ["//visibility:public"],
)
```

### croutine/（含汇编文件）

```python
cc_library(
    name = "croutine",
    srcs = glob(["*.cpp"]) + ["swap_x86_64.S"],
    hdrs = glob(["*.h"]),
    includes = [".."],
    deps = ["//common"],
    visibility = ["//visibility:public"],
)
```

> **说明**：`swap_x86_64.S` 是 x86_64 架构的协程上下文切换汇编代码，Bazel 原生支持 `.S` 文件。

### data/ — 纯头文件

```python
cc_library(
    name = "data",
    hdrs = glob(["**/*.h"]),
    includes = [".."],
    visibility = ["//visibility:public"],
)
```

### task/

```python
cc_library(
    name = "task",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    includes = [".."],
    deps = ["//common", "//logger"],
    visibility = ["//visibility:public"],
)
```

### transport/（含多个子目录）

```python
cc_library(
    name = "transport_common",
    srcs = glob(["common/*.cpp"]),
    hdrs = glob(["common/*.h"]),
    includes = [".."],
    deps = ["//base", "//common", "//logger", "//serialize"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "transport_message",
    srcs = glob(["message/*.cpp"]),
    hdrs = glob(["message/*.h"]),
    includes = [".."],
    deps = [":transport_common", "//common", "//logger"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "transport_qos",
    srcs = glob(["qos/*.cpp"]),
    hdrs = glob(["qos/*.h"]),
    includes = [".."],
    deps = ["//common", "//config"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "transport_shm",
    srcs = glob(["shm/*.cpp"]),
    hdrs = glob(["shm/*.h"]),
    includes = [".."],
    deps = ["//base", "//common", "//event", "//logger", ":transport_common"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "transport_rtps",
    srcs = glob(["rtps/*.cpp"]),
    hdrs = glob(["rtps/*.h"]),
    includes = [".."],
    deps = [
        "//common", "//logger", "//config",
        ":transport_common", ":transport_qos",
        "@fastrtps",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "transport_receiver",
    hdrs = glob(["receiver/*.h"]),
    includes = [".."],
    deps = [":transport_message", ":transport_shm", ":transport_rtps"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "transport_transmitter",
    hdrs = glob(["transmitter/*.h"]),
    includes = [".."],
    deps = [":transport_message", ":transport_shm", ":transport_rtps"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "transport_dispatcher",
    srcs = glob(["dispatcher/*.cpp"]),
    hdrs = glob(["dispatcher/*.h"]),
    includes = [".."],
    deps = [":transport_message", ":transport_shm", ":transport_rtps", "//common", "//logger"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "transport",
    srcs = ["transport.cpp"],
    hdrs = ["transport.h"],
    includes = [".."],
    deps = [
        ":transport_common", ":transport_message", ":transport_qos",
        ":transport_shm", ":transport_rtps", ":transport_receiver",
        ":transport_transmitter", ":transport_dispatcher",
        "//common", "//config", "//event", "//logger",
    ],
    visibility = ["//visibility:public"],
)
```

### scheduler/（含子目录 common/, policy/）

```python
cc_library(
    name = "scheduler",
    srcs = glob(["*.cpp", "common/*.cpp", "policy/*.cpp"]),
    hdrs = glob(["*.h", "common/*.h", "policy/*.h"]),
    includes = [".."],
    deps = [
        "//common", "//config", "//croutine", "//data", "//logger", "//time",
    ],
    visibility = ["//visibility:public"],
)
```

### discovery/（含子目录 communication/, container/, role/, specific_manager/）

```python
cc_library(
    name = "discovery",
    srcs = glob(["*.cpp", "**/*.cpp"]),
    hdrs = glob(["*.h", "**/*.h"]),
    includes = [".."],
    deps = [
        "//base", "//common", "//config", "//logger", "//time",
        "//transport:transport_message", "//transport:transport_qos",
        "//transport:transport_rtps",
        "@fastrtps",
    ],
    visibility = ["//visibility:public"],
)
```

### node/

```python
cc_library(
    name = "node",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    includes = [".."],
    deps = [
        "//blocker", "//common", "//config", "//croutine",
        "//data", "//discovery", "//event", "//logger",
        "//scheduler", "//transport",
    ],
    visibility = ["//visibility:public"],
)
```

### 根目录 — init/state

```python
# 根目录 BUILD 文件
package(default_visibility = ["//visibility:public"])

cc_library(
    name = "init",
    srcs = ["init.cpp"],
    hdrs = ["init.h"],
    includes = ["."],
    deps = ["//common", "//discovery", "//logger", "//node", "//scheduler", "//transport"],
)

cc_library(
    name = "state",
    srcs = ["state.cpp"],
    hdrs = ["state.h"],
    includes = ["."],
    deps = ["//common", "//logger"],
)
```

---

## 测试文件 include 修正

部分测试文件使用 `#include "../xxx"` 相对路径，需要统一改为 `#include "xxx"`。

在 `m_test/` 目录下执行：

```bash
cd m_test

# 修正 init.h
sed -i 's|#include "\.\./init\.h"|#include "init.h"|g' test_*.cpp

# 修正 node/node.h
sed -i 's|#include "\.\./node/node\.h"|#include "node/node.h"|g' test_*.cpp

# 修正 serialize/serializable.h
sed -i 's|#include "\.\./serialize/serializable\.h"|#include "serialize/serializable.h"|g' test_*.cpp

# 修正 base/ 头文件
sed -i 's|#include "\.\./base/thread_pool\.h"|#include "base/thread_pool.h"|g' test_*.cpp
sed -i 's|#include "\.\./base/object_pool\.h"|#include "base/object_pool.h"|g' test_*.cpp
sed -i 's|#include "\.\./base/concurrent_object_pool\.h"|#include "base/concurrent_object_pool.h"|g' test_*.cpp
sed -i 's|#include "\.\./base/signal_slot\.h"|#include "base/signal_slot.h"|g' test_*.cpp
sed -i 's|#include "\.\./base/atomic_rw_lock\.h"|#include "base/atomic_rw_lock.h"|g' test_*.cpp
```

需要修改的文件列表：

| 文件 | 行号 | 原始 include |
|------|------|-------------|
| test_main.cpp | 15-17 | `"../init.h"`, `"../node/node.h"`, `"../serialize/serializable.h"` |
| test_hello_pub.cpp | 8-10 | 同上 |
| test_hello_sub.cpp | 8-10 | 同上 |
| test_thread_pool.cpp | 5 | `"../base/thread_pool.h"` |
| test_object_pool.cpp | 20-21 | `"../base/object_pool.h"`, `"../base/concurrent_object_pool.h"` |
| test_signal.cpp | 2 | `"../base/signal_slot.h"` |
| test_atomic_rw_lock.cpp | 4 | `"../base/atomic_rw_lock.h"` |
| test_object_pool1.cpp | 1 | `"../base/object_pool.h"` |

---

## m_test/BUILD

```python
TEST_SRCS = glob(["test_*.cpp"])

[cc_test(
    name = src.replace(".cpp", ""),
    srcs = [src],
    includes = [".."],
    deps = [
        "//:init",
        "//:state",
        "//base",
        "//common",
        "//config",
        "//croutine",
        "//data",
        "//discovery",
        "//event",
        "//logger",
        "//node",
        "//scheduler",
        "//serialize",
        "//task",
        "//time",
        "//transport",
        "@com_google_googletest//:gtest_main",
        "@json//:nlohmann_json",
        "@fastrtps",
    ],
) for src in TEST_SRCS]
```

---

## 验证构建

```bash
# 检查 Bazel 是否安装
bazel version

# 首次构建（会下载并编译第三方依赖，耗时较长）
bazel build //...

# 构建单个模块
bazel build //logger:logger
bazel build //node:node
bazel build //transport:transport

# 如果头文件找不到，用 --sandbox_debug 调试
bazel build //logger:logger --sandbox_debug

# 运行全部测试
bazel test //m_test/...

# 运行单个测试
bazel test //m_test:test_main

# 查看测试输出
bazel test //m_test:test_main --test_output=all
```

### 常见问题排查

| 问题 | 解决方案 |
|------|---------|
| `sha256 mismatch` | 将 Bazel 输出的正确 sha256 填入 WORKSPACE |
| `header not found` | 检查 `includes = [".."]` 是否正确，或用 `--sandbox_debug` 查看 sandbox 目录结构 |
| FastDDS 编译失败 | 检查系统是否安装了 cmake：`sudo apt install cmake` |
| 链接错误 `undefined reference` | 检查 `.bazelrc` 中是否缺少系统库的 `--linkopt` |
| `.S` 汇编文件编译错误 | 确认 `croutine/BUILD` 中 `srcs` 包含了 `"swap_x86_64.S"` |
