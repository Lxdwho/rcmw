/**
 * @brief 协程工厂
 * @date 2025.12.27
 */

#include <memory>
#include <utility>
#include "rcmw/common/global_data.h"
#include "rcmw/logger/log.h"
#include "rcmw/croutine/croutine.h"
#include "rcmw/data/data_visitor/data_visitor.h"
#include "rcmw/event/perf_event_cache.h"

#ifndef _RCMW_CROUTINE_FACTORY_H_
#define _RCMW_CROUTINE_FACTORY_H_

namespace hnu       {
namespace rcmw      {
namespace croutine  {

/**
 * @brief 协程工厂: 维护了一个visitor以及一个函数包装器
 */
class RoutineFactory {
public:
    using VoidFunc = std::function<void()>;
    using CreateRoutineFunc = std::function<VoidFunc()>;

    CreateRoutineFunc create_routine;

    inline std::shared_ptr<data::DataVisitorBase> GetDataVisitor() const {
        return data_visitor_;
    }

    inline void SetDataVisitor(const std::shared_ptr<data::DataVisitorBase>& dv) {
        data_visitor_ = dv;
    }
private:
    std::shared_ptr<data::DataVisitorBase> data_visitor_ = nullptr;
};

/**
 * @brief 协程工厂创建函数: 实例化协程工厂，及其内部变量
 * @tparam Mx 数据类型？
 * @tparam F 可调用对象模板
 */
template<typename M0, typename F>
RoutineFactory CreateRoutineFactory(F&& f, const std::shared_ptr<data::DataVisitor<M0>>& dv) {
    RoutineFactory factory;
    factory.SetDataVisitor(dv);
    factory.create_routine = [=]() {
        return [=]() {
            std::shared_ptr<M0> msg;
            for(;;) {
                CRoutine::GetCurrentRoutine()->set_state(RoutineState::DATA_WAIT);
                if(dv->TryFetch(msg)) {
                    f(msg);
                    CRoutine::Yield(RoutineState::READY);
                }
                else {
                    CRoutine::Yield();
                }
            }
        };
    };
    return factory;
}

/**
 * @brief 协程工厂创建函数: 实例化协程工厂，及其内部变量
 * @tparam Mx 数据类型？
 * @tparam F 可调用对象模板
 */
template<typename M0, typename M1, typename F>
RoutineFactory CreateRoutineFactory(F&& f, const std::shared_ptr<data::DataVisitor<M0, M1>>& dv) {
    RoutineFactory factory;
    factory.SetDataVisitor(dv);
    factory.create_routine = [=]() {
        return [=]() {
            std::shared_ptr<M0> msg0;
            std::shared_ptr<M1> msg1;
            for(;;) {
                CRoutine::GetCurrentRoutine()->set_state(RoutineState::DATA_WAIT);
                if(dv->TryFetch(msg0, msg1)) {
                    f(msg0, msg1);
                    CRoutine::Yield(RoutineState::READY);
                }
                else {
                    CRoutine::Yield();
                }
            }
        };
    };
    return factory;
}

/**
 * @brief 协程工厂创建函数: 实例化协程工厂，及其内部变量
 * @tparam Mx 数据类型？
 * @tparam F 可调用对象模板
 */
template<typename M0, typename M1, typename M2, typename F>
RoutineFactory CreateRoutineFactory(F&& f, const std::shared_ptr<data::DataVisitor<M0, M1, M2>>& dv) {
    RoutineFactory factory;
    factory.SetDataVisitor(dv);
    factory.create_routine = [=]() {
        return [=]() {
            std::shared_ptr<M0> msg0;
            std::shared_ptr<M1> msg1;
            std::shared_ptr<M2> msg2;
            for(;;) {
                CRoutine::GetCurrentRoutine()->set_state(RoutineState::DATA_WAIT);
                if(dv->TryFetch(msg0, msg1, msg2)) {
                    f(msg0, msg1, msg2);
                    CRoutine::Yield(RoutineState::READY);
                }
                else {
                    CRoutine::Yield();
                }
            }
        };
    };
    return factory;
}

/**
 * @brief 协程工厂创建函数: 实例化协程工厂，及其内部变量
 * @tparam Mx 数据类型？
 * @tparam F 可调用对象模板
 */
template<typename M0, typename M1, typename M2, typename M3, typename F>
RoutineFactory CreateRoutineFactory(F&& f, const std::shared_ptr<data::DataVisitor<M0, M1, M2, M3>>& dv) {
    RoutineFactory factory;
    factory.SetDataVisitor(dv);
    factory.create_routine = [=]() {
        return [=]() {
            std::shared_ptr<M0> msg0;
            std::shared_ptr<M1> msg1;
            std::shared_ptr<M2> msg2;
            std::shared_ptr<M3> msg3;
            for(;;) {
                CRoutine::GetCurrentRoutine()->set_state(RoutineState::DATA_WAIT);
                if(dv->TryFetch(msg0, msg1, msg2, msg3)) {
                    f(msg0, msg1, msg2, msg3);
                    CRoutine::Yield(RoutineState::READY);
                }
                else {
                    CRoutine::Yield();
                }
            }
        };
    };
    return factory;
}

/**
 * @brief 协程工厂创建函数: 实例化协程工厂，及其内部变量
 * @tparam Function 可调用对象模板
 */
template <typename Function>
RoutineFactory CreateRoutineFactory(Function&& f) {
    RoutineFactory factory;
    factory.create_routine = [f = std::forward<Function&&>(f)]() { return f; };
    return factory;
}

} // croutine 
} // rcmw
} // hnu

#endif
