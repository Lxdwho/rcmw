#include <rcmw/node/publisher.h>
#include <rcmw/config/RoleAttributes.h>
#include <rcmw/common/global_data.h>
#include <rcmw/serialize/serializable.h>
#include <rcmw/serialize/data_stream.h>
#include <vector>
#include <rcmw/init.h>
using namespace hnu::rcmw;


struct TestMsg : public Serializable
{
    uint64_t timestamp;  

    uint64_t image;
    SERIALIZE(timestamp,image)
};


namespace george
{
    using namespace config;
    void test_rtps_pub()
    {
        config::RoleAttributes role_attr;
        role_attr.channel_name = "exampletopic";
        role_attr.node_name = "publisher";
        role_attr.channel_id =common::GlobalData::RegisterChannel("exampletopic");
        
        std::cout << std::boolalpha;
        int n = 0;
        TestMsg testmsg;
        testmsg.timestamp = 0;


        Publisher<TestMsg> publisher(role_attr);
        std::cout<<"Init publisher " << publisher.Init()<< std::endl;

        std::shared_ptr<TestMsg> msg_ptr = std::make_shared<TestMsg>(testmsg);
        while (1)
        {
        publisher.Publish(msg_ptr);
        std::cout << "Publisher seq: " << n << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        n++;
        }
    }
}

namespace Timer{
    void test_pub(){
        config::RoleAttributes role_attr;
        role_attr.channel_name = "exampletopic";
        role_attr.node_name = "publisher";
        role_attr.channel_id =common::GlobalData::RegisterChannel("exampletopic");

        Publisher<ChangeMsg> publisher(role_attr);
        std::cout<<"Init publisher " << publisher.Init()<< std::endl;
        ChangeMsg testmsg;
        testmsg.timestamp = 1;
        std::shared_ptr<ChangeMsg> msg_ptr = std::make_shared<ChangeMsg>(testmsg);
        int n = 0;
        while (1)
        {
            publisher.Publish(msg_ptr);
            std::cout << "Publisher seq: " << n << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            n++;
            msg_ptr->timestamp = n;
        }
    }
}
int main()
{
    hnu::rcmw::Init("PublisherTest");
    //george::test_rtps_pub();
    Timer::test_pub();
    return 0;
}