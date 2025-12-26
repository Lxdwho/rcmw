/**
 * @brief for_each：对容器中前END-1个数与END进行比较，如果可以比较
 * @date  2025.11.05
 */

#ifndef _FOR_EACH_H_
#define _FOR_EACH_H_

#include <type_traits>
#include "rcmw/base/macros.h"
#include <iostream>

namespace hnu  {
namespace rcmw {
namespace base {

/* 得到HasLess用于判断是否存在运算符‘<’ */
DEFINE_TYPE_TRAIT(HasLess, operator<)

/* 存在比较运算‘<’， 返回bool */
template <typename Value, typename End>
typename std::enable_if<   
    HasLess<Value>::value && HasLess<End>::value, bool>::type
LessThan(const Value& val, const End& end) {
    return val < end;
}

/* 不存在比较运算‘<’， 返回是否相等 */
template <typename Value, typename End>
typename std::enable_if<
    !HasLess<Value>::value || !HasLess<End>::value, bool>::type
LessThan(const Value& val, const End& end) {
    return val != end;
}

/* 对容器进行遍历比较 */
#define FOR_EACH(i, begin, end) \
    for(auto i = (true ? (begin) : (end)); \
        hnu::rcmw::base::LessThan(i, (end)); ++i)

} // base
} // rcmw
} // hnu

#endif
