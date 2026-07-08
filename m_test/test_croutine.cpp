#include "rcmw/croutine/croutine.h"
#include "rcmw/common/global_data.h"
#include "gtest/gtest.h"

#include <atomic>
#include <chrono>
#include <thread>

using namespace hnu::rcmw::croutine;
using namespace hnu::rcmw::common;

class CRoutineTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger_Init("test_croutine.log");
        GlobalData::Instance();
    }
};

// ==================== 基础属性测试 ====================

TEST_F(CRoutineTest, InitialState) {
    CRoutine cr([]{});
    EXPECT_EQ(cr.state(), RoutineState::READY);
    EXPECT_EQ(cr.id(), 0);
    EXPECT_EQ(cr.name(), "");
    EXPECT_EQ(cr.processor_id(), -1);
    EXPECT_EQ(cr.priority(), 0);
    EXPECT_EQ(cr.group_name(), "");
}

TEST_F(CRoutineTest, SettersAndGetters) {
    CRoutine cr([]{});

    auto id = GlobalData::RegisterTaskName("test_task");
    cr.set_id(id);
    cr.set_name("test_task");
    cr.set_processor_id(2);
    cr.set_priority(10);
    cr.set_group_name("group_a");

    EXPECT_EQ(cr.id(), id);
    EXPECT_EQ(cr.name(), "test_task");
    EXPECT_EQ(cr.processor_id(), 2);
    EXPECT_EQ(cr.priority(), 10);
    EXPECT_EQ(cr.group_name(), "group_a");
}

TEST_F(CRoutineTest, SetAndGetState) {
    CRoutine cr([]{});

    cr.set_state(RoutineState::DATA_WAIT);
    EXPECT_EQ(cr.state(), RoutineState::DATA_WAIT);

    cr.set_state(RoutineState::IO_WAIT);
    EXPECT_EQ(cr.state(), RoutineState::IO_WAIT);

    cr.set_state(RoutineState::SLEEP);
    EXPECT_EQ(cr.state(), RoutineState::SLEEP);

    cr.set_state(RoutineState::FINISHED);
    EXPECT_EQ(cr.state(), RoutineState::FINISHED);

    cr.set_state(RoutineState::READY);
    EXPECT_EQ(cr.state(), RoutineState::READY);
}

// ==================== 状态转换测试 ====================

TEST_F(CRoutineTest, WakeSetsReady) {
    CRoutine cr([]{});

    cr.set_state(RoutineState::DATA_WAIT);
    EXPECT_EQ(cr.state(), RoutineState::DATA_WAIT);

    cr.Wake();
    EXPECT_EQ(cr.state(), RoutineState::READY);
}

TEST_F(CRoutineTest, HangUpSetsDataWait) {
    CRoutine cr([]{});

    cr.set_state(RoutineState::READY);
    cr.HangUp();
    EXPECT_EQ(cr.state(), RoutineState::DATA_WAIT);
}

TEST_F(CRoutineTest, UpdateStateFromSleep) {
    CRoutine cr([]{
        CRoutine::GetCurrentRoutine()->Sleep(std::chrono::seconds(100));
    });

    cr.set_state(RoutineState::READY);
    cr.Resume();  // 进入 Sleep，wake_time_ 设为 100 秒后

    // 尚未到唤醒时间，保持 SLEEP
    EXPECT_EQ(cr.UpdateState(), RoutineState::SLEEP);
}

TEST_F(CRoutineTest, UpdateStateFromDataWait) {
    CRoutine cr([]{});

    cr.set_state(RoutineState::DATA_WAIT);
    cr.SetUpdateFlag();  // clear updated_ flag
    EXPECT_EQ(cr.UpdateState(), RoutineState::READY);
}

TEST_F(CRoutineTest, UpdateStateFromIOWait) {
    CRoutine cr([]{});

    cr.set_state(RoutineState::IO_WAIT);
    cr.SetUpdateFlag();
    EXPECT_EQ(cr.UpdateState(), RoutineState::READY);
}

TEST_F(CRoutineTest, UpdateStateOnlyOnce) {
    CRoutine cr([]{});

    // 第一次 UpdateState 会转换状态
    cr.set_state(RoutineState::DATA_WAIT);
    cr.SetUpdateFlag();
    EXPECT_EQ(cr.UpdateState(), RoutineState::READY);

    // 第二次不再转换（updated_ 已被 test_and_set）
    cr.set_state(RoutineState::DATA_WAIT);
    EXPECT_EQ(cr.UpdateState(), RoutineState::DATA_WAIT);
}

TEST_F(CRoutineTest, SetUpdateFlagResets) {
    CRoutine cr([]{});

    // 模拟：UpdateState 消费了 flag，再 SetUpdateFlag 重置
    cr.set_state(RoutineState::DATA_WAIT);
    cr.SetUpdateFlag();
    cr.UpdateState();  // 消费 flag

    cr.set_state(RoutineState::IO_WAIT);
    cr.SetUpdateFlag();  // 重置 flag
    EXPECT_EQ(cr.UpdateState(), RoutineState::READY);
}

// ==================== 锁测试 ====================

TEST_F(CRoutineTest, AcquireRelease) {
    CRoutine cr([]{});

    EXPECT_TRUE(cr.Acquire());
    EXPECT_FALSE(cr.Acquire());  // 已被持有
    cr.Release();
    EXPECT_TRUE(cr.Acquire());   // 释放后可再次获取
    cr.Release();
}

TEST_F(CRoutineTest, AcquireFromMultipleThreads) {
    CRoutine cr([]{});
    std::atomic<int> counter{0};

    EXPECT_TRUE(cr.Acquire());

    std::thread t([&]{
        if (cr.Acquire()) {
            counter++;
            cr.Release();
        }
    });

    // 主线程持有锁期间，子线程无法获取
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(counter.load(), 0);

    cr.Release();
    t.join();

    // 释放后子线程可以获取
    // 注意：t 可能已经结束，这里只验证锁机制
}

// ==================== 上下文测试 ====================

TEST_F(CRoutineTest, ContextNotNull) {
    CRoutine cr([]{});
    EXPECT_NE(cr.GetContext(), nullptr);
    EXPECT_NE(cr.GetStack(), nullptr);
}

TEST_F(CRoutineTest, StackInitializedAfterConstruct) {
    CRoutine cr([]{});
    // MakeContext 已初始化 sp，不为 nullptr
    EXPECT_NE(*(cr.GetStack()), nullptr);
}

TEST_F(CRoutineTest, MainStackAccess) {
    auto main_stack = CRoutine::GetMainStack();
    EXPECT_NE(main_stack, nullptr);
}

// ==================== 协程执行测试 ====================

TEST_F(CRoutineTest, ResumeRunsFunction) {
    std::atomic<bool> ran{false};

    CRoutine cr([&]{ ran = true; });
    cr.set_id(GlobalData::RegisterTaskName("resume_test"));
    cr.set_state(RoutineState::READY);

    cr.Resume();
    EXPECT_TRUE(ran.load());
}

TEST_F(CRoutineTest, ResumeToFinished) {
    CRoutine cr([]{});
    cr.set_state(RoutineState::READY);

    auto state = cr.Resume();
    EXPECT_EQ(state, RoutineState::FINISHED);
}

TEST_F(CRoutineTest, YieldSetsState) {
    RoutineState yielded_state = RoutineState::READY;

    CRoutine cr([&]{
        yielded_state = CRoutine::GetCurrentRoutine()->state();
        CRoutine::Yield(RoutineState::IO_WAIT);
    });
    cr.set_state(RoutineState::READY);

    cr.Resume();
    // Yield 后协程状态应为 IO_WAIT
    EXPECT_EQ(cr.state(), RoutineState::IO_WAIT);
}

TEST_F(CRoutineTest, GetCurrentRoutineInExecution) {
    CRoutine* current = nullptr;

    CRoutine cr([&]{
        current = CRoutine::GetCurrentRoutine();
        CRoutine::Yield();
    });
    cr.set_state(RoutineState::READY);

    cr.Resume();
    EXPECT_EQ(current, &cr);
}

TEST_F(CRoutineTest, MultipleResumeAndYield) {
    int count = 0;

    CRoutine cr([&]{
        count++;
        CRoutine::Yield(RoutineState::IO_WAIT);
        count++;
        CRoutine::Yield(RoutineState::IO_WAIT);
        count++;
    });
    cr.set_state(RoutineState::READY);

    cr.Resume();
    EXPECT_EQ(count, 1);
    EXPECT_EQ(cr.state(), RoutineState::IO_WAIT);

    cr.set_state(RoutineState::READY);
    cr.Resume();
    EXPECT_EQ(count, 2);
    EXPECT_EQ(cr.state(), RoutineState::IO_WAIT);

    cr.set_state(RoutineState::READY);
    cr.Resume();
    EXPECT_EQ(count, 3);
    EXPECT_EQ(cr.state(), RoutineState::FINISHED);
}

// ==================== Sleep 测试 ====================

TEST_F(CRoutineTest, SleepSetsWakeTime) {
    CRoutine cr([]{
        CRoutine::GetCurrentRoutine()->Sleep(std::chrono::milliseconds(50));
    });
    cr.set_state(RoutineState::READY);

    auto before = std::chrono::steady_clock::now();
    cr.Resume();

    EXPECT_EQ(cr.state(), RoutineState::SLEEP);
    EXPECT_GT(cr.wake_time(), before);
}

TEST_F(CRoutineTest, SleepWakesAfterDuration) {
    CRoutine cr([]{
        CRoutine::GetCurrentRoutine()->Sleep(std::chrono::milliseconds(50));
    });
    cr.set_state(RoutineState::READY);
    cr.Resume();
    EXPECT_EQ(cr.state(), RoutineState::SLEEP);

    // 未到时间，UpdateState 保持 SLEEP
    EXPECT_EQ(cr.UpdateState(), RoutineState::SLEEP);

    // 等待超时
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(cr.UpdateState(), RoutineState::READY);
}

// ==================== Stop 测试 ====================

TEST_F(CRoutineTest, StopSetsForceStop) {
    CRoutine cr([]{
        while (true) {
            CRoutine::Yield(RoutineState::IO_WAIT);
        }
    });
    cr.set_state(RoutineState::READY);

    // Resume 后协程进入 IO_WAIT
    cr.Resume();
    EXPECT_EQ(cr.state(), RoutineState::IO_WAIT);

    // Stop 标记后，再次 Resume 应该结束
    cr.Stop();
    cr.set_state(RoutineState::READY);
    auto state = cr.Resume();
    EXPECT_EQ(state, RoutineState::FINISHED);
}

// ==================== Run 测试 ====================

TEST_F(CRoutineTest, RunExecutesDirectly) {
    std::atomic<bool> ran{false};

    CRoutine cr([&]{ ran = true; });
    cr.Run();
    EXPECT_TRUE(ran.load());
}

// ==================== 综合场景测试 ====================

TEST_F(CRoutineTest, FullLifecycle) {
    std::vector<int> execution_order;

    CRoutine cr([&]{
        execution_order.push_back(1);
        CRoutine::Yield(RoutineState::DATA_WAIT);

        execution_order.push_back(2);
        CRoutine::Yield(RoutineState::IO_WAIT);

        execution_order.push_back(3);
    });

    auto id = GlobalData::RegisterTaskName("lifecycle_test");
    cr.set_id(id);
    cr.set_name("lifecycle_test");
    cr.set_priority(5);
    cr.set_processor_id(0);

    // 初始 READY
    EXPECT_EQ(cr.state(), RoutineState::READY);

    // 第一次执行
    cr.Resume();
    EXPECT_EQ(execution_order.size(), 1);
    EXPECT_EQ(cr.state(), RoutineState::DATA_WAIT);

    // 唤醒并第二次执行
    cr.Wake();
    EXPECT_EQ(cr.state(), RoutineState::READY);
    cr.Resume();
    EXPECT_EQ(execution_order.size(), 2);
    EXPECT_EQ(cr.state(), RoutineState::IO_WAIT);

    // 唤醒并第三次执行
    cr.Wake();
    cr.Resume();
    EXPECT_EQ(execution_order.size(), 3);
    EXPECT_EQ(cr.state(), RoutineState::FINISHED);

    // 验证执行顺序
    EXPECT_EQ(execution_order[0], 1);
    EXPECT_EQ(execution_order[1], 2);
    EXPECT_EQ(execution_order[2], 3);
}

TEST_F(CRoutineTest, DataWaitToReadyViaUpdateState) {
    std::atomic<int> runs{0};

    CRoutine cr([&]{
        runs++;
        CRoutine::Yield(RoutineState::DATA_WAIT);
        runs++;
    });
    cr.set_state(RoutineState::READY);

    cr.Resume();
    EXPECT_EQ(runs.load(), 1);
    EXPECT_EQ(cr.state(), RoutineState::DATA_WAIT);

    // 通过 UpdateState 转换到 READY
    cr.SetUpdateFlag();
    EXPECT_EQ(cr.UpdateState(), RoutineState::READY);

    cr.Resume();
    EXPECT_EQ(runs.load(), 2);
    EXPECT_EQ(cr.state(), RoutineState::FINISHED);
}

TEST_F(CRoutineTest, MultipleYieldWithDifferentStates) {
    std::vector<RoutineState> states;

    CRoutine cr([&]{
        CRoutine::Yield(RoutineState::DATA_WAIT);
        CRoutine::Yield(RoutineState::IO_WAIT);
        CRoutine::Yield(RoutineState::SLEEP);
    });
    cr.set_state(RoutineState::READY);

    cr.Resume();
    states.push_back(cr.state());

    cr.set_state(RoutineState::READY);
    cr.Resume();
    states.push_back(cr.state());

    cr.set_state(RoutineState::READY);
    cr.Resume();
    states.push_back(cr.state());

    EXPECT_EQ(states[0], RoutineState::DATA_WAIT);
    EXPECT_EQ(states[1], RoutineState::IO_WAIT);
    EXPECT_EQ(states[2], RoutineState::SLEEP);
}

// ==================== 入口 ====================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
