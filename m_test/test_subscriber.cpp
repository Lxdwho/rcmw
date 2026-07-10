#include "rcmw/node/subscriber.h"
#include <rcmw/config/RoleAttributes.h>
#include <rcmw/transport/message/message_info.h>
#include <rcmw/config/unit_test.h>
#include <gtest/gtest.h>
#include <rcmw/common/global_data.h>
#include <rcmw/init.h>
using namespace hnu::rcmw;
using namespace hnu::rcmw::common;

struct TestMsg : public Serializable
{
    uint64_t timestamp;  

    uint64_t image;
    SERIALIZE(timestamp,image)
};


void test_sub(std::string node_name){
        config::RoleAttributes role_attr0, role_attr1;
        
        role_attr0.channel_name = "exampletopic";
        role_attr0.node_name = node_name;
        role_attr0.channel_id =common::GlobalData::RegisterChannel("exampletopic");

        role_attr1.channel_name = "exampletopic";
        role_attr1.node_name = node_name + "1";
        role_attr1.channel_id =common::GlobalData::RegisterChannel("exampletopic");

        Subscriber<TestMsg> subscriber0(
                role_attr0, [](const std::shared_ptr<TestMsg>& msg){
                    std::cout << "timestamp0 is "<< msg->timestamp<< std::endl;
                });

        Subscriber<TestMsg> subscriber1(
                role_attr1, [](const std::shared_ptr<TestMsg>& msg){
                    std::cout << "timestamp1 is "<< msg->timestamp<< std::endl;
                });

        std::cout<< "Init subscriber0 " << std::boolalpha << subscriber0.Init() << std::endl;
        std::cout<< "Init subscriber1 " << std::boolalpha << subscriber1.Init() << std::endl;

        while (1)
        {
            /* code */
        }
}

int main(int argc, char* argv[])
{
    hnu::rcmw::Init("SubscriberTest");
    // Logger::Get_instance()->Set_console(false);
    GlobalData::Instance()->SetProcessGroup("example_sched_classic");
    Logger::Get_instance()->level(Logger::LOG_INFO);
    if(argc > 1) {
        std::string str = std::string(argv[1]);
        test_sub(str);
    }
    else test_sub("Subscriber");
    return 0;
}
