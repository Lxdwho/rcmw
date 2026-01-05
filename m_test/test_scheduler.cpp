#include <rcmw/scheduler/scheduler.h>
#include <rcmw/config/scheduler_conf.h>
#include <rcmw/common/global_data.h>
#include <rcmw/init.h>
#include <rcmw/scheduler/processor_context.h>
#include <rcmw/scheduler/scheduler_factory.h>
#include <gtest/gtest.h>
#include <vector>
#include <rcmw/base/for_each.h>
#include <rcmw/scheduler/processor.h>
#include <rcmw/scheduler/policy/classic_context.h>
#include <future>

using namespace hnu::rcmw::common;
using namespace hnu::rcmw::croutine;

using namespace hnu::rcmw;

void scheduler_testfunc() {
    std::cout << "test cheduler func --------------------------------" << std::endl;
}
void proc() {
    std::cout << "test pro func --------------------------------" << std::endl;
}

// TEST(test_sched_classic, classic) {
//     GlobalData::Instance()->SetProcessGroup("example_sched_classic");
//     auto sched1 = dynamic_cast<scheduler::SchedulerClassic*>(scheduler::Instance());
//     std::shared_ptr<CRoutine> cr = std::make_shared<CRoutine>(scheduler_testfunc);
//     auto task_id = GlobalData::RegisterTaskName("ABC");
//     cr->set_id(task_id);
//     cr->set_name("ABC");

//     EXPECT_TRUE(sched1->DispatchTask(cr));
//     // dispatch the same task
//     EXPECT_FALSE(sched1->DispatchTask(cr));
//     EXPECT_TRUE(sched1->RemoveTask("ABC"));

//     std::shared_ptr<CRoutine> cr1 = std::make_shared<CRoutine>(scheduler_testfunc);

//     cr1->set_id(GlobalData::RegisterTaskName("xxxxxx"));
//     cr1->set_name("xxxxxx");
//     EXPECT_TRUE(sched1->DispatchTask(cr1));

//     auto t = std::thread(scheduler_testfunc);
//     sched1->SetInnerThreadAttr("shm", &t);
//     if (t.joinable()) {
//         t.join();
//     }
//     ADEBUG << "-----------Finished Test_scheduler---------";
// }

TEST(test_creat_classic, classic) {
    GlobalData::Instance()->SetProcessGroup("example_sched_classic");
    //   auto sched = scheduler::Instance();
    auto sched = dynamic_cast<scheduler::SchedulerClassic*>(scheduler::Instance());
    // std::shared_ptr<CRoutine> cr = std::make_shared<CRoutine>(scheduler_testfunc);
    // auto task_id1 = GlobalData::RegisterTaskName("ABC");
    // cr->set_id(task_id1);
    // cr->set_name("ABC");

    // EXPECT_TRUE(sched->DispatchTask(cr));


    // read example_sched_classic.conf task 'ABC' prio
    std::string croutine_name = "A";

    EXPECT_TRUE(sched->CreateTask(&proc, croutine_name));
    //create a croutine with the same name
    EXPECT_FALSE(sched->CreateTask(&proc, croutine_name));

    auto task_id = GlobalData::RegisterTaskName(croutine_name);
    EXPECT_TRUE(sched->NotifyTask(task_id));

    EXPECT_TRUE(sched->RemoveTask(croutine_name));
    // remove the same task twice
    EXPECT_FALSE(sched->RemoveTask(croutine_name));
    // remove a not exist task
    EXPECT_FALSE(sched->RemoveTask("driver"));
    sched->Shutdown();
}

int main(int argc, char** argv) {
    std::string logfile_name = "test_scheduler.log";
    Logger_Init(logfile_name);
    Logger::Get_instance()->Set_console(false);
    auto global_data = common::GlobalData::Instance();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
