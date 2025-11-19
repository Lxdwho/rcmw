/**
 * @brief 
 * @date 2025.11.18
 */

#ifndef _UNIT_TEST_H_
#define _UNIT_TEST_H_

#include <string>
#include "rcmw/serialize/data_stream.h"
#include "rcmw/serialize/serializable.h"

namespace hnu    {
namespace rcmw   {
namespace config {

using namespace serialize;

struct UnitTest : public Serializable
{
    std::string class_name;
    std::string case_name;
    SERIALIZE(class_name,case_name)
};

struct Chatter : public Serializable{
    uint64_t timestamp;
    uint64_t lidar_timestamp;
    uint64_t seq;
    std::string content;
    SERIALIZE(timestamp,lidar_timestamp,seq,content)
};

} // config
} // rcmw
} // hnu

#endif
