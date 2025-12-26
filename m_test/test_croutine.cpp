#include "rcmw/croutine/croutine.h"
#include "rcmw/common/global_data.h"
#include "gtest/gtest.h"

using namespace hnu::rcmw::croutine;
using namespace hnu::rcmw::common;

void function() { 
    ADEBUG << "I'm Running!";
    CRoutine::Yield(RoutineState::IO_WAIT); 
}

TEST(Croutine, croutinetest) {
    std::string logfile_name = "test_croutine.log";
    Logger_Init(logfile_name);

    //初始化 global_data
    auto global_data = GlobalData::Instance();
    // hnu::cmw::Init("croutine_test");
    std::shared_ptr<CRoutine> cr = std::make_shared<CRoutine>(function);

    auto id = GlobalData::RegisterTaskName("croutine");

    cr->set_id(id);
    cr->set_name("croutine");
    cr->set_processor_id(0);
    cr->set_priority(1);
    cr->set_state(RoutineState::DATA_WAIT);

    EXPECT_EQ(cr->state(), RoutineState::DATA_WAIT);
    cr->Wake();
    EXPECT_EQ(cr->state(), RoutineState::READY);

    cr->UpdateState();
    EXPECT_EQ(cr->state(), RoutineState::READY);
    EXPECT_EQ(*(cr->GetMainStack()), nullptr);
    cr->Resume();
    EXPECT_NE(*(cr->GetMainStack()), nullptr);
    EXPECT_EQ(cr->state(), RoutineState::IO_WAIT);
    cr->Stop();
    EXPECT_EQ(cr->Resume(), RoutineState::FINISHED);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
