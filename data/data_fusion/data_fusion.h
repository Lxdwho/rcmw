/**
 * @brief 数据融合？
 * @date 2025.12.26
 */

#ifndef _RCMW_DATA_DATA_FUSION_H_
#define _RCMW_DATA_DATA_FUSION_H_

#include <deque>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>
#include "rcmw/common/types.h"

namespace hnu   {
namespace rcmw  {
namespace data  {
namespace fusion{

template <typename M0, typename M1 = NullType, typename M2 = NullType, typename M3 = NullType>
class DataFusion {
public:
    virtual ~DataFusion() {}
    virtual bool Fusion(uint64_t* index, std::shared_ptr<M0>& m0, 
                                         std::shared_ptr<M1>& m1, 
                                         std::shared_ptr<M2>& m2, 
                                         std::shared_ptr<M3>& m3) = 0;
};

template <typename M0, typename M1, typename M2>
class DataFusion<M0, M1, M2> {
public:
    virtual ~DataFusion() {}
    virtual bool Fusion(uint64_t* index, std::shared_ptr<M0>& m0, 
                                         std::shared_ptr<M1>& m1, 
                                         std::shared_ptr<M2>& m2) = 0;
};

template <typename M0, typename M1>
class DataFusion<M0, M1> {
public:
    virtual ~DataFusion() {}
    virtual bool Fusion(uint64_t* index, std::shared_ptr<M0>& m0, 
                                         std::shared_ptr<M1>& m1) = 0;
};

} // fusion
} // data
} // rcmw
} // hnu

#endif
