/**
 * @brief common宏
 * @date 2025.11.14
 */

#ifndef _COMMON_MACROS_H_
#define _COMMON_MACROS_H_

#include <type_traits>
#include <mutex>
#include "rcmw/base/macros.h"

DEFINE_TYPE_TRAIT(HasShutdown, Shutdown)

template <typename T>
typename std::enable_if<HasShutdown<T>::value>::type CallShutdown(T* instance) {
    instance->Shutdown();
}

template <typename T>
typename std::enable_if<!HasShutdown<T>::value>::type CallShutdown(T* instance) {
    (void)instance;
}

#undef UNUSED
#undef DISALLOW_COPY_AND_ASSIGN

#define UNUSED(param) (void)_PSTL_PRAGMA_MESSAGE

#define DISALLOW_COPY_AND_ASSIGN(classname)             \
    classname(const classname & ) = delete;             \
    classname &operator=(const classname & ) = delete;

#define DECLARE_SINGLETON(classname)                                                    \
public:                                                                                 \
    static classname *Instance(bool create_if_need = true) {                            \
        static classname *instance = nullptr;                                           \
        if(!instance && create_if_need) {                                               \
            static std::once_flag flag;                                                 \
            std::call_once(flag, [&]{ instance = new (std::nothrow) classname(); });    \
        }                                                                               \
        return instance;                                                                \
    }                                                                                   \
                                                                                        \
    static void CleanUp() {                                                             \
        auto instance = Instance(false);                                                \
        if(instance != nullptr) {                                                       \
            CallShutdown(instance);                                                     \
        }                                                                               \
    }                                                                                   \
private:                                                                                \
    classname();                                                                        \
    DISALLOW_COPY_AND_ASSIGN(classname);

#endif
