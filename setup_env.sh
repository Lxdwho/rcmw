#!/bin/bash
# ============================================================
# rcmw 一键环境配置脚本
# 自动检查环境、安装依赖、编译 FastDDS / GTest
#
# 用法: chmod +x setup_env.sh && ./setup_env.sh
# ============================================================

set -e

# ---------- 颜色 ----------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

info()  { echo -e "${GREEN}[INFO]${NC} $*"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $*"; }
error() { echo -e "${RED}[ERROR]${NC} $*"; }

# ---------- 安装路径 ----------
FASTDDS_PREFIX="$HOME/Fast-DDS/install"
GTEST_PREFIX="$HOME/googletest/install"
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"

# ============================================================
# 1. 环境检查
# ============================================================
check_env() {
    info "========== 环境检查 =========="

    # 检查 Ubuntu
    if [ ! -f /etc/os-release ]; then
        error "未检测到 Ubuntu 系统"
        exit 1
    fi
    . /etc/os-release
    info "系统: $PRETTY_NAME"

    # 检查 GCC 版本
    if command -v gcc &>/dev/null; then
        GCC_VER=$(gcc -dumpversion)
        info "GCC: $GCC_VER"
    else
        warn "GCC 未安装，将在后续步骤安装"
    fi

    # 检查 CMake 版本
    if command -v cmake &>/dev/null; then
        CMAKE_VER=$(cmake --version | head -1 | grep -oP '\d+\.\d+\.\d+')
        info "CMake: $CMAKE_VER"
    else
        warn "CMake 未安装，将在后续步骤安装"
    fi

    # 检查 git
    if ! command -v git &>/dev/null; then
        warn "Git 未安装，将在后续步骤安装"
    fi

    info "环境检查完成"
    echo ""
}

# ============================================================
# 2. 安装系统依赖
# ============================================================
install_system_deps() {
    info "========== 安装系统依赖 =========="

    sudo apt update

    sudo apt install -y \
        build-essential \
        cmake \
        git \
        wget \
        unzip \
        libasio-dev \
        libtinyxml2-dev \
        uuid-dev \
        libssl-dev \
        libcurl4-openssl-dev

    info "系统依赖安装完成"
    echo ""
}

# ============================================================
# 3. 安装 foonathan_memory
# ============================================================
install_foonathan_memory() {
    info "========== 安装 foonathan_memory =========="

    mkdir -p "$HOME/Fast-DDS"
    cd "$HOME/Fast-DDS"

    if [ -d "foonathan_memory_vendor" ]; then
        warn "foonathan_memory_vendor 已存在，跳过下载"
    else
        git clone https://github.com/eProsima/foonathan_memory_vendor.git
    fi

    cd foonathan_memory_vendor
    mkdir -p build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="$FASTDDS_PREFIX" -DBUILD_SHARED_LIBS=ON
    make -j$(nproc)
    make install

    # 清理源码和 build
    cd "$HOME/Fast-DDS"
    rm -rf foonathan_memory_vendor

    info "foonathan_memory 安装完成"
    echo ""
}

# ============================================================
# 4. 安装 FastCDR
# ============================================================
install_fastcdr() {
    info "========== 安装 FastCDR v1.0.15 =========="

    cd "$HOME/Fast-DDS"

    if [ -d "Fast-CDR" ]; then
        warn "Fast-CDR 已存在，跳过下载"
    else
        git clone https://github.com/eProsima/Fast-CDR.git
    fi

    cd Fast-CDR
    git checkout v1.0.15
    mkdir -p build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="$FASTDDS_PREFIX"
    make -j$(nproc)
    make install

    # 清理源码和 build
    cd "$HOME/Fast-DDS"
    rm -rf Fast-CDR

    info "FastCDR 安装完成"
    echo ""
}

# ============================================================
# 5. 安装 FastDDS
# ============================================================
install_fastdds() {
    info "========== 安装 FastDDS v2.3.0 =========="

    cd "$HOME/Fast-DDS"

    if [ -d "Fast-DDS" ]; then
        warn "Fast-DDS 已存在，跳过下载"
    else
        git clone --recursive https://github.com/eProsima/Fast-DDS.git
    fi

    cd Fast-DDS
    git checkout v2.3.0
    mkdir -p build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="$FASTDDS_PREFIX" -DCMAKE_PREFIX_PATH="$FASTDDS_PREFIX"
    make -j$(nproc)
    make install

    # 清理源码和 build
    cd "$HOME/Fast-DDS"
    rm -rf Fast-DDS

    info "FastDDS 安装完成"
    echo ""
}

# ============================================================
# 6. 安装 GTest (v1.12.0, 支持 C++14)
# ============================================================
install_gtest() {
    info "========== 安装 GTest v1.12.0 =========="

    cd "$HOME"

    if [ -d "googletest" ]; then
        warn "googletest 已存在，跳过下载"
    else
        git clone https://github.com/google/googletest.git
    fi

    cd googletest
    git checkout v1.12.0
    mkdir -p build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="$GTEST_PREFIX" -DBUILD_SHARED_LIBS=OFF
    make -j$(nproc)
    make install

    # 只删源码和 build，保留 install 目录
    cd "$HOME"
    rm -rf googletest/build
    rm -rf googletest/.git
    rm -rf googletest/googletest googletest/googlemock
    rm -rf googletest/CMakeLists.txt googletest/README.md

    info "GTest 安装完成"
    echo ""
}

# ============================================================
# 7. 设置环境变量
# ============================================================
setup_env_vars() {
    info "========== 设置环境变量 =========="

    BASHRC="$HOME/.bashrc"
    MARKER="# === rcmw FastDDS env ==="

    # 先删除旧的配置
    if grep -q "$MARKER" "$BASHRC" 2>/dev/null; then
        sed -i "/$MARKER/,/$MARKER/d" "$BASHRC"
        warn "已清理 .bashrc 中的旧配置"
    fi

    cat >> "$BASHRC" << 'EOF'
# === rcmw FastDDS env ===
export PATH=$PATH:$HOME/Fast-DDS/install/bin
export LD_LIBRARY_PATH=$HOME/Fast-DDS/install/lib:$LD_LIBRARY_PATH
# === rcmw FastDDS env ===
EOF

    info "环境变量已写入 $BASHRC"
    info "执行 'source ~/.bashrc' 或重新打开终端生效"
    echo ""
}

# ============================================================
# 8. 验证安装
# ============================================================
verify_install() {
    info "========== 验证安装 =========="

    local ok=0
    local fail=0

    # 检查 FastDDS
    if [ -f "$FASTDDS_PREFIX/lib/libfastrtps.so" ]; then
        info "✓ libfastrtps.so"
        ok=$((ok+1))
    else
        error "✗ libfastrtps.so 未找到"
        fail=$((fail+1))
    fi

    if [ -f "$FASTDDS_PREFIX/lib/libfastcdr.so" ]; then
        info "✓ libfastcdr.so"
        ok=$((ok+1))
    else
        error "✗ libfastcdr.so 未找到"
        fail=$((fail+1))
    fi

    if [ -f "$FASTDDS_PREFIX/lib/libfoonathan_memory"*.so 2>/dev/null ]; then
        info "✓ libfoonathan_memory.so"
        ok=$((ok+1))
    else
        error "✗ libfoonathan_memory.so 未找到"
        fail=$((fail+1))
    fi

    # 检查 GTest
    if [ -f "$GTEST_PREFIX/lib/libgtest.a" ]; then
        info "✓ libgtest.a"
        ok=$((ok+1))
    else
        error "✗ libgtest.a 未找到"
        fail=$((fail+1))
    fi

    # 检查头文件
    if [ -d "$FASTDDS_PREFIX/include/fastrtps" ]; then
        info "✓ FastDDS 头文件"
        ok=$((ok+1))
    else
        error "✗ FastDDS 头文件未找到"
        fail=$((fail+1))
    fi

    if [ -d "$GTEST_PREFIX/include/gtest" ]; then
        info "✓ GTest 头文件"
        ok=$((ok+1))
    else
        error "✗ GTest 头文件未找到"
        fail=$((fail+1))
    fi

    echo ""
    info "验证完成: ${ok} 通过, ${fail} 失败"

    if [ $fail -gt 0 ]; then
        error "部分组件安装失败，请检查上方日志"
        return 1
    fi

    info "所有组件安装成功！"
    echo ""
}

# ============================================================
# 9. 编译测试（可选）
# ============================================================
build_test() {
    info "========== 编译测试 =========="

    cd "$PROJECT_ROOT/m_test"
    if make test_lock_benchmark -j$(nproc); then
        info "测试编译成功: m_test/test_lock_benchmark.out"
        ls -lh test_lock_benchmark.out
    else
        error "测试编译失败"
        return 1
    fi
    echo ""
}

# ============================================================
# 主流程
# ============================================================
main() {
    echo ""
    info "=========================================="
    info "  rcmw 环境一键配置"
    info "  安装路径: FastDDS → $FASTDDS_PREFIX"
    info "           GTest  → $GTEST_PREFIX"
    info "=========================================="
    echo ""

    check_env
    install_system_deps
    install_foonathan_memory
    install_fastcdr
    install_fastdds
    install_gtest
    setup_env_vars
    verify_install
    build_test

    info "=========================================="
    info "  环境配置完成！"
    info "  执行 'source ~/.bashrc' 使环境变量生效"
    info "=========================================="
}

# 支持选择性安装
case "${1:-all}" in
    all)                main ;;
    check)              check_env ;;
    deps)               install_system_deps ;;
    foonathan)          install_foonathan_memory ;;
    fastcdr)            install_fastcdr ;;
    fastdds)            install_fastdds ;;
    gtest)              install_gtest ;;
    verify)             verify_install ;;
    test)               build_test ;;
    *)
        echo "用法: $0 [all|check|deps|foonathan|fastcdr|fastdds|gtest|verify|test]"
        echo ""
        echo "  all        - 完整安装（默认）"
        echo "  check      - 仅检查环境"
        echo "  deps       - 仅安装系统依赖"
        echo "  foonathan  - 仅安装 foonathan_memory"
        echo "  fastcdr    - 仅安装 FastCDR"
        echo "  fastdds    - 仅安装 FastDDS"
        echo "  gtest      - 仅安装 GTest"
        echo "  verify     - 仅验证安装"
        echo "  test       - 仅编译测试"
        ;;
esac
