#include <iostream>

using namespace std;
#include "rcmw/serialize/data_stream.h"
#include "rcmw/serialize/serializable.h"
#include <vector>
#include "rcmw/config/topology_change.h"
#include "rcmw/config/qos_profile.h"
using namespace hnu::rcmw::serialize;

using namespace hnu::rcmw::config;
int main()
{

    DataStream ds;

    // Data_test data;
    // data.a = "test";
    // data.role = ROLE_CLIENT;
    // data.b = 5;
    // data.c = 500;


    // Data_pb data_test;
    // data_test.b = data;
    // data_test.c = "test";
    // ds << data_test;

    // Data_pb data1;

    // ds >> data1;
    // data1.b.show();
    // data1.show();

    ChangeMsg change_msg;
    change_msg.change_type = CHANGE_NODE;
    change_msg.operate_type = OPT_JOIN;
    change_msg.role_type = ROLE_WRITER;
    ds << change_msg;
    ChangeMsg change_msg1;
    DataStream ds1(ds.data(), ds.ByteSize());
    ds1 >> change_msg1;
    std::cout << "fx:" << change_msg1.change_type
              << " fy:" << change_msg1.operate_type
              << " fz " << change_msg1.role_type << std::endl;
    ChangeMsg change_msg2;
    DataStream ds2(ds.data(), ds.ByteSize());
    ds2 >> change_msg2;
    std::cout << "fx:" << change_msg2.change_type
              << " fy:" << change_msg2.operate_type
              << " fz " << change_msg2.role_type << std::endl;

    return 0;
}
