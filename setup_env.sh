#!/bin/bash
# ============================================================
# rcmw 一键环境配置脚本
# 自动检查环境、安装依赖、编译 FastDDS / GTest
#
# 用法:
#   ./setup_env.sh              # 完整安装到 thirdparty/
#   ./setup_env.sh --prefix /path/to/dir  # 指定安装目录
#   ./setup_env.sh gtest        # 仅安装 GTest
#   ./setup_env.sh --help       # 查看帮助
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

# ---------- 项目根目录 ----------
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# ---------- 解析 --prefix 参数 ----------
INSTALL_PREFIX=""
ACTION="all"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --help|-h)
            echo "用法: $0 [--prefix 安装目录] [动作]"
            echo ""
            echo "  --prefix DIR   指定安装目录（默认: 项目 thirdparty/）"
            echo ""
            echo "动作:"
            echo "  all        - 完整安装（默认）"
            echo "  check      - 仅检查环境"
            echo "  deps       - 仅安装系统依赖"
            echo "  foonathan  - 仅安装 foonathan_memory"
            echo "  fastcdr    - 仅安装 FastCDR"
            echo "  fastdds    - 仅安装 FastDDS"
            echo "  gtest      - 仅安装 GTest"
            echo "  verify     - 仅验证安装"
            echo "  test       - 仅编译测试"
            exit 0
            ;;
        *)
            ACTION="$1"
            shift
            ;;
    esac
done

# 默认安装到 thirdparty/
if [ -z "$INSTALL_PREFIX" ]; then
    INSTALL_PREFIX="$PROJECT_ROOT/thirdparty"
fi

FASTDDS_PREFIX="$INSTALL_PREFIX/Fast-DDS"
GTEST_PREFIX="$INSTALL_PREFIX/googletest"

# 临时编译目录
BUILD_TMP="$PROJECT_ROOT/.build_tmp"

# ============================================================
# 1. 环境检查
# ============================================================
check_env() {
    info "========== 环境检查 =========="

    if [ ! -f /etc/os-release ]; then
        error "未检测到 Ubuntu 系统"
        exit 1
    fi
    # shellcheck source=/dev/null
    . /etc/os-release
    info "系统: $PRETTY_NAME"

    if command -v gcc &>/dev/null; then
        info "GCC: $(gcc -dumpversion)"
    else
        warn "GCC 未安装，将在后续步骤安装"
    fi

    if command -v cmake &>/dev/null; then
        info "CMake: $(cmake --version | head -1 | grep -oP '\d+\.\d+\.\d+')"
    else
        warn "CMake 未安装，将在后续步骤安装"
    fi

    if ! command -v git &>/dev/null; then
        warn "Git 未安装，将在后续步骤安装"
    fi

    info "安装目录: $INSTALL_PREFIX"
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

    mkdir -p "$BUILD_TMP" && cd "$BUILD_TMP"

    if [ -d "foonathan_memory_vendor" ]; then
        warn "foonathan_memory_vendor 已存在，跳过下载"
    else
        git clone https://github.com/eProsima/foonathan_memory_vendor.git
    fi

    cd foonathan_memory_vendor
    mkdir -p build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="$FASTDDS_PREFIX" -DBUILD_SHARED_LIBS=ON

    make -j"$(nproc)"
    make install

    info "foonathan_memory 安装完成"
    echo ""
}

# ============================================================
# 4. 安装 FastCDR
# ============================================================
install_fastcdr() {
    info "========== 安装 FastCDR v1.0.15 =========="

    mkdir -p "$BUILD_TMP" && cd "$BUILD_TMP"

    if [ -d "Fast-CDR" ]; then
        warn "Fast-CDR 已存在，跳过下载"
    else
        git clone https://github.com/eProsima/Fast-CDR.git
    fi

    cd Fast-CDR
    git checkout v1.0.15
    mkdir -p build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="$FASTDDS_PREFIX"
    make -j"$(nproc)"
    make install

    info "FastCDR 安装完成"
    echo ""
}

# ============================================================
# 5. 安装 FastDDS
# ============================================================
install_fastdds() {
    info "========== 安装 FastDDS v2.3.0 =========="

    mkdir -p "$BUILD_TMP" && cd "$BUILD_TMP"

    if [ -d "Fast-DDS" ]; then
        warn "Fast-DDS 已存在，跳过下载"
    else
        git clone --recursive https://github.com/eProsima/Fast-DDS.git
    fi

    cd Fast-DDS
    git checkout v2.3.0
    mkdir -p build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="$FASTDDS_PREFIX" -DCMAKE_PREFIX_PATH="$FASTDDS_PREFIX"
    make -j"$(nproc)"
    make install

    info "FastDDS 安装完成"
    echo ""
}

# ============================================================
# 6. 安装 GTest (v1.12.0, 支持 C++14)
# ============================================================
install_gtest() {
    info "========== 安装 GTest v1.12.0 =========="

    mkdir -p "$BUILD_TMP" && cd "$BUILD_TMP"

    if [ -d "googletest" ]; then
        warn "googletest 已存在，跳过下载"
    else
        git clone https://github.com/google/googletest.git
    fi

    cd googletest
    git checkout v1.12.0
    mkdir -p build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="$GTEST_PREFIX" -DBUILD_SHARED_LIBS=OFF
    make -j"$(nproc)"
    make install

    info "GTest 安装完成"
    echo ""
}

# ============================================================
# 7. 清理临时编译目录
# ============================================================
cleanup_build() {
    info "========== 清理临时编译文件 =========="

    if [ -d "$BUILD_TMP" ]; then
        rm -rf "$BUILD_TMP"
        info "已清理 $BUILD_TMP"
    fi

    # 只有安装到项目 thirdparty/ 时才清理 install 中的非必要文件
    DEFAULT_PREFIX="$PROJECT_ROOT/thirdparty"
    if [ "$INSTALL_PREFIX" = "$DEFAULT_PREFIX" ]; then
        for dir in "$FASTDDS_PREFIX" "$GTEST_PREFIX"; do
            if [ -d "$dir" ]; then
                cd "$dir"
                rm -rf bin share
                rm -rf lib/cmake lib/pkgconfig
                info "已清理 $dir 中的非必要文件"
            fi
        done
    fi

    echo ""
}

# ============================================================
# 8. 设置环境变量
# ============================================================
setup_env_vars() {
    info "========== 设置环境变量 =========="

    BASHRC="$HOME/.bashrc"
    MARKER="# === rcmw env ==="

    # 先删除旧的配置
    if grep -q "$MARKER" "$BASHRC" 2>/dev/null; then
        sed -i "/$MARKER/,/$MARKER/d" "$BASHRC"
        warn "已清理 .bashrc 中的旧配置"
    fi

    cat >> "$BASHRC" << EOF
# === rcmw env ===
export RCMW_PATH=$PROJECT_ROOT
export FASTDDS_PREFIX=$FASTDDS_PREFIX
export GTEST_PREFIX=$GTEST_PREFIX
export LD_LIBRARY_PATH=$FASTDDS_PREFIX/lib:\$LD_LIBRARY_PATH
# === rcmw env ===
EOF

    info "环境变量已写入 $BASHRC"
    info "执行 'source ~/.bashrc' 或重新打开终端生效"
    echo ""
}

# ============================================================
# 9. 验证安装
# ============================================================
verify_install() {
    info "========== 验证安装 =========="

    local ok=0
    local fail=0

    check_file() {
        local label="$1" path="$2"
        if [ -e "$path" ]; then
            info "✓ $label"
            ok=$((ok+1))
        else
            error "✗ $label 未找到: $path"
            fail=$((fail+1))
        fi
    }

    check_file "libfastrtps.so"        "$FASTDDS_PREFIX/lib/libfastrtps.so"
    check_file "libfastcdr.so"         "$FASTDDS_PREFIX/lib/libfastcdr.so"
    # shellcheck disable=SC2086
    check_file "libfoonathan_memory"   $FASTDDS_PREFIX/lib/libfoonathan_memory*.so
    check_file "libgtest.a"            "$GTEST_PREFIX/lib/libgtest.a"
    check_file "libgtest_main.a"       "$GTEST_PREFIX/lib/libgtest_main.a"
    check_file "FastDDS 头文件"        "$FASTDDS_PREFIX/include/fastrtps"
    check_file "FastCDR 头文件"        "$FASTDDS_PREFIX/include/fastcdr"
    check_file "GTest 头文件"          "$GTEST_PREFIX/include/gtest"

    echo ""
    info "验证完成: ${ok} 通过, ${fail} 失败"

    if [ $fail -gt 0 ]; then
        error "部分组件安装失败，请检查上方日志"
        return 1
    fi

    # 显示安装目录大小
    local fastdds_size
    fastdds_size=$(du -sh "$FASTDDS_PREFIX" 2>/dev/null | cut -f1)
    local gtest_size
    gtest_size=$(du -sh "$GTEST_PREFIX" 2>/dev/null | cut -f1)
    info "FastDDS 安装大小: $fastdds_size"
    info "GTest  安装大小: $gtest_size"
    info "所有组件安装成功！"
    echo ""
}

# ============================================================
# 10. 编译测试（可选）
# ============================================================
build_test() {
    info "========== 编译测试 =========="

    cd "$PROJECT_ROOT/m_test"
    if make test_lock_benchmark -j"$(nproc)"; then
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
    info "  安装目录: $INSTALL_PREFIX"
    info "=========================================="
    echo ""

    check_env
    install_system_deps
    install_foonathan_memory
    install_fastcdr
    install_fastdds
    install_gtest
    cleanup_build
    setup_env_vars
    verify_install
    build_test

    info "=========================================="
    info "  环境配置完成！"
    info "  执行 'source ~/.bashrc' 使环境变量生效"
    info "=========================================="
}

# 分发动作
case "$ACTION" in
    all)        main ;;
    check)      check_env ;;
    deps)       install_system_deps ;;
    foonathan)  install_foonathan_memory ;;
    fastcdr)    install_fastcdr ;;
    fastdds)    install_fastdds ;;
    gtest)      install_gtest ;;
    verify)     verify_install ;;
    test)       build_test ;;
    cleanup)    cleanup_build ;;
    *)
        echo "未知动作: $ACTION"
        echo "执行 '$0 --help' 查看帮助"
        exit 1
        ;;
esac
