#include "notifier_factory.h"
#include "rcmw/logger/log.h"
#include "condition_notifier.h"
#include "multicast_notifier.h"

using namespace hnu::rcmw::transport;

int main()
{
    Logger_Init("log.txt");
    NotifierBase* ptr = NotifierFactory::CreateNotifier();
    ConditionNotifier* nptr = dynamic_cast<ConditionNotifier*>(ptr);
    nptr->Shutdown();
    nptr->~ConditionNotifier();
    nptr->CleanUp();
    nptr = nullptr;
    return 0;
}

// g++ ./*.cpp ../../logger/logger.cpp -I/home/me/cmw_ws/ -I/home/me/cmw_ws/rcmw/thirdparty/ -o test_notify_factory.out
