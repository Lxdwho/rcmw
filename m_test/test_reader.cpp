#include "rcmw/common/global_data.h"
#include "rcmw/config/RoleAttributes.h"
#include "rcmw/transport/dispatcher/rtps_dispatcher.h"
#include "rcmw/common/util.h"
#include "rcmw/transport/receiver/receiver.h"
#include "rcmw/transport/tranport.h"
#include "rcmw/config/topology_change.h"
#include "rcmw/logger/log.h"


using namespace hnu::rcmw::transport;
using namespace hnu::rcmw::common;
using namespace hnu::rcmw::config;

using ReceiverPtr = std::shared_ptr<Receiver<std::string>>;

void TEST_GLOBAL_DATA()
{
    using namespace hnu::rcmw::common;
    std::cout <<"--------------------------Global_Data Test--------------------------" << std::endl;
    std::cout <<"HostIP: " << GlobalData::Instance()->HostIp() << std::endl;
    std::cout <<"HostName: " << GlobalData::Instance()->HostName() << std::endl;
    std::cout <<"ProcessId: " << GlobalData::Instance()->ProcessId() << std::endl;
    std::cout <<"ProcessGroup: " << GlobalData::Instance()->ProcessGroup() << std::endl;
}

void TEST_ChangeMsg()
{
    RoleAttributes attr;
    attr.channel_name = "exampleTopic";
    attr.host_name = GlobalData::Instance()->HostName();
    attr.host_ip = GlobalData::Instance()->HostIp();
    attr.process_id =  GlobalData::Instance()->ProcessId();
    attr.channel_id = GlobalData::Instance()->RegisterChannel(attr.channel_name);
    QosProfile qos;
    attr.qos_profile = qos;
    auto listener1 = [](const std::shared_ptr<ChangeMsg>& message ,
                       const MessageInfo& info, const RoleAttributes&){
                        
                        std::cout<<"time: " << message->timestamp << 
                        " operate_type:"  << message->operate_type << 
                        " seq:" << info.seq_num() << std::endl;
                        
                       };
    auto rtps1 =Transport::Instance()->CreateReceiver<ChangeMsg>(attr,listener1, OptionalMode::SHM);
    printf("Press Enter to stop the Reader.\n");
    std::cin.ignore();
}   

int main()
{
    Logger_Init("reader.log");
    AERROR << "test";
    // Logger::Instance()->level(Logger::LOG_INFO);
    TEST_GLOBAL_DATA();
    TEST_ChangeMsg();
    //TEST_MUTILISTENER();
    return 0;
}
