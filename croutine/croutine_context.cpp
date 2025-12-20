/**
 * @brief 协程栈
 * @date 2025.12.20
 */

#include "rcmw/croutine/croutine_context.h"

namespace hnu       {
namespace rcmw      {
namespace croutine  {

/*
低地址（栈向下生长方向 ← ）
├─ ctx->stack  (缓冲区起始)
│
│     STACK ELSE     ← 运行时栈真正向下扩展的区域
│   (Reserved)       ← 预留但尚未使用
│
├─ ctx->sp  ──→  指向这里
│   +------------------+
│   |       RBP        |  56 B  寄存器快照（跳板）
│   +------------------+  ↑
│   |       ...        |  │
│   +------------------+  │  ← 只被 pop 一次，不再生长
│   |       R12        |  │
│   +------------------+  │
│   |       R13        |  │
│   +------------------+  │
│   |       RDI        |  │  ← popq %rdi 时载入 arg
│   +------------------+  │
│   |  Return Address  |  │  ← ret 时跳入 f1
│   +------------------+  ↓
│
└─ 缓冲区末尾 (高地址)
*/

void MakeContext(const func & f1, const void * arg, RoutineContext * ctx) {
    ctx->sp = ctx->stack + STACK_SIZE - 2 * sizeof(void*) - REGISTERS_SIZE;
    std::memset(ctx->sp, 0, REGISTERS_SIZE);
#ifdef __aarch64__
    char *sp = ctx->stack + STACK_SIZE - sizeof(void *);
#else
    char *sp = ctx->stack + STACK_SIZE - 2 * sizeof(void*);
#endif
    *reinterpret_cast<void **>(sp) = reinterpret_cast<void *>(f1);
    sp -= sizeof(void *);
    *reinterpret_cast<void **>(sp) = const_cast<void *>(arg);
}

} // croutine
} // rcmw
} // hnu
