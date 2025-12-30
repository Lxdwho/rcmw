/**
 * @brief 协程栈
 * @date 2025.12.20
 */

#ifndef _RCMW_CROUTINE_CONTEXT_H_
#define _RCMW_CROUTINE_CONTEXT_H_

#include <cstdlib>
#include <cstring>
#include <iostream>
#include "rcmw/logger/log.h"

extern "C" {
    extern void ctx_swap(void**, void**) asm("ctx_swap");
}

namespace hnu       {
namespace rcmw      {
namespace croutine  {

constexpr size_t STACK_SIZE = 2 * 1024 * 1024;

#if defined __aarch64__
constexpr size_t REGISTERS_SIZE = 160;
#else
constexpr size_t REGISTERS_SIZE = 56;
#endif

/**
 * @brief 协程执行体
 * @return void
 * @param void*
 */
typedef void(*func)(void*);

/**
 * @brief 协程栈结构体
 */
struct RoutineContext {
    char stack[STACK_SIZE];
    char* sp = nullptr;
#if defined __aarch64__
} __attribute__((aligned(16)));
#else
};
#endif

/**
 * @brief 协程上下文构建
 * @param func 协程执行体
 * @param arg  协程执行传入参数
 * @param ctx  协程上下文结构体
 * @return void
 */
void MakeContext(const func& f1, const void* arg, RoutineContext* ctx);

/**
 * @brief 协程上下文切换
 * @param src_sp 当前sp指针
 * @param dest_sp 切换的协程的sp指针
 * @return void
 */
inline void SwapContext(char** src_sp, char** dest_sp) {
    ctx_swap(reinterpret_cast<void**>(src_sp), reinterpret_cast<void**>(dest_sp));
}

} // croutine
} // rcmw
} // hnu

#endif
