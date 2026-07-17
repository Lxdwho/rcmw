# ============================================================
# 自动发现 test_*.cpp 并生成对应目标的 Makefile
# 所有 .o 文件统一输出到 obj/ 目录，保留目录结构
#
# 用法:
#   make <Tab>          -- 自动补全所有 test_ 目标
#   make test_foo       -- 编译并生成 test_foo.out
#   make all            -- 编译全部
#   make clean          -- 清理 .out .log 和 obj/
#   make list           -- 列出所有可用的测试目标
# ============================================================

# 编译器和选项
CC      := g++
CFLAGS  := -std=c++14 -MMD -MP
DEBUG   := -g
INCLUDE := -I$(CURDIR)/../thirdparty/Fast-DDS/include \
           -I$(CURDIR)/../thirdparty/googletest/include \
           -I../../ \
           -I../ \
           -I../thirdparty/

ThirdPartyLibPath := -L$(CURDIR)/../thirdparty/Fast-DDS/lib \
                     -L$(CURDIR)/../thirdparty/googletest/lib

ThirdPartyLib := -lfastrtps -lfastcdr -lfoonathan_memory-0.7.4 \
                 -lpthread -luuid -lrt -latomic -lgtest -lgtest_main -ldl

# ---------- 自动发现测试文件 ----------
TEST_SRCS  := $(wildcard test_*.cpp)
TARGETS    := $(TEST_SRCS:.cpp=)

# ---------- 非测试源文件（父目录中的 .cpp / .S）----------
SRCS := $(shell find .. -type f -name '*.cpp' ! -path '*/m_test/*' ! -path './m_test/*' ! -path '*/thirdparty/*' ! -name 'test_*.cpp')
ASMS := $(shell find .. -type f -name '*.S' ! -path '*/m_test/*' ! -path './m_test/*' ! -path '*/thirdparty/*')

# ---------- .o 统一输出到 obj/，保留目录结构 ----------
# ../scheduler/scheduler.cpp → obj/scheduler/scheduler.o
# ../transport/shm/state.cpp → obj/transport/shm/state.o
# 避免不同目录下同名文件冲突（如 state.cpp）
OBJ_DIR := obj
LIB_OBJS := $(patsubst ../%, $(OBJ_DIR)/%, $(SRCS:.cpp=.o)) \
            $(patsubst ../%, $(OBJ_DIR)/%, $(ASMS:.S=.o))
TEST_OBJS := $(addprefix $(OBJ_DIR)/, $(TEST_SRCS:.cpp=.o))
ALL_OBJS := $(LIB_OBJS) $(TEST_OBJS)

# 自动生成的依赖文件 (.d)，与 .o 同目录
DEPS := $(ALL_OBJS:.o=.d)

# 静态库：把所有库 .o 打包成 .a，链接时只拉取用到的 .o
LIB_A := $(OBJ_DIR)/librcmw.a

# 需要创建的子目录
OBJ_DIRS := $(sort $(dir $(ALL_OBJS)))

# ============================================================
# 默认目标
# ============================================================
.DEFAULT_GOAL := help

.PHONY: help all clean cleanout cleanobj list $(TARGETS)

help:
	@echo "可用测试目标 (共 $(words $(TARGETS)) 个):"
	@$(foreach t,$(TARGETS),echo "  make $(t)";)
	@echo ""
	@echo "其他命令:"
	@echo "  make all        - 编译全部测试"
	@echo "  make clean      - 清理所有产物 (obj/ .out .log)"
	@echo "  make cleanout   - 仅清理 .out .log"
	@echo "  make cleanobj   - 仅清理 obj/"
	@echo "  make list       - 仅列出目标名 (用于补全)"

list:
	@$(foreach t,$(TARGETS),echo "$(t)";)

all: $(TARGETS)

# ============================================================
# 打包静态库
# ============================================================
$(LIB_A): $(LIB_OBJS) | $(OBJ_DIRS)
	ar rcs $@ $^

# ============================================================
# 链接规则：每个 test 链接自己的 .o + 静态库（只拉取用到的 .o）
# ============================================================
$(TARGETS): %: $(OBJ_DIR)/%.o $(LIB_A) | $(OBJ_DIRS)
	$(CC) $(DEBUG) -o $@.out $< -L$(OBJ_DIR) -lrcmw $(ThirdPartyLibPath) $(ThirdPartyLib)

# ============================================================
# 编译规则：源文件 → obj/ 下的 .o
# -MMD -MP 自动生成 .d 依赖文件，头文件改了只重编受影响的 .o
# ============================================================
$(OBJ_DIR)/%.o: ../%.cpp | $(OBJ_DIRS)
	$(CC) $(INCLUDE) $(CFLAGS) $(DEBUG) -c -o $@ $<

$(OBJ_DIR)/%.o: ../%.S | $(OBJ_DIRS)
	$(AS) -c $< -o $@

# 测试文件：m_test/*.cpp → obj/*.o
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIRS)
	$(CC) $(INCLUDE) $(CFLAGS) $(DEBUG) -c -o $@ $<

# 自动创建 obj/ 子目录
$(OBJ_DIRS):
	@mkdir -p $@

# ============================================================
# 包含自动生成的依赖文件（首次编译时 .d 不存在，- 前缀忽略错误）
# ============================================================
-include $(DEPS)

# ============================================================
# 清理
# ============================================================
clean:
	rm -rf *.out *.log $(OBJ_DIR)

cleanout:
	rm -rf *.out *.log

cleanobj:
	rm -rf $(OBJ_DIR)
