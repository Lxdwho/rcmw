#!/bin/bash
# ============================================================
# rcmw 环境配置脚本
# 设置相关环境变量
#
# 用法:
#   source ev.sh              # 在当前 shell 生效
#   ./ev.sh                   # 仅打印，不生效（子 shell）
#
# 说明:
#   运行前请先执行 setup_env.sh 安装 FastDDS 和 GTest
# ============================================================

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

info()  { echo -e "${GREEN}[INFO]${NC} $*"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $*"; }
error() { echo -e "${RED}[ERROR]${NC} $*"; }

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# ============================================================
# 1. 项目路径
# ============================================================
export RCMW_PATH="$PROJECT_ROOT"
info "RCMW_PATH = $RCMW_PATH"

# ============================================================
# 2. 第三方库路径（FastDDS / GTest）
#    优先使用 setup_env.sh 安装到 thirdparty/ 的版本
#    如果第三方库安装在其他位置，修改此处
# ============================================================
FASTDDS_PREFIX="${FASTDDS_PREFIX:-$PROJECT_ROOT/thirdparty/Fast-DDS}"
GTEST_PREFIX="${GTEST_PREFIX:-$PROJECT_ROOT/thirdparty/googletest}"

export FASTDDS_PREFIX
export GTEST_PREFIX
info "FASTDDS_PREFIX = $FASTDDS_PREFIX"
info "GTEST_PREFIX   = $GTEST_PREFIX"

# ============================================================
# 3. 库搜索路径
# ============================================================
export LD_LIBRARY_PATH="$FASTDDS_PREFIX/lib:$GTEST_PREFIX/lib:$LD_LIBRARY_PATH"
info "LD_LIBRARY_PATH 已更新"

# ============================================================
# 4. 头文件搜索路径（可选，Makefile 中已硬编码）
#    如需使用，取消注释
# ============================================================
# export CPLUS_INCLUDE_PATH="$FASTDDS_PREFIX/include:$GTEST_PREFIX/include:$PROJECT_ROOT:$CPLUS_INCLUDE_PATH"

# ============================================================
# 5. RCMW_IP - FastDDS 网络绑定地址
#    默认 127.0.0.1（本地回环）
#    多机通信时设置为实际网卡 IP
# ============================================================
export RCMW_IP="${RCMW_IP:-127.0.0.1}"
info "RCMW_IP = $RCMW_IP"

# ============================================================
# 6. 共享内存路径（可选，默认 /dev/shm）
# ============================================================
# export ISOLATED_SHM_PATH="/dev/shm"

echo ""
info "环境变量设置完成"
warn "如果是直接执行 ./ev.sh，请改用 'source ev.sh' 使变量在当前 shell 生效"
