#include <gtest/gtest.h>
#include "../lib/scheduler.h"

float addOne(float x) { return x + 1; }
float mul(float a, float b) { return a * b; }
float constant() { return 42.0f; }

struct MyClass {
    float scale;
    float mul(float x) const { return x * scale; }
};

TEST(TTaskSchedulerTest, ZeroArgTask) {
    TTaskScheduler scheduler;
    auto id = scheduler.add(constant);
    scheduler.executeAll();
    EXPECT_FLOAT_EQ(scheduler.getResult<float>(id), 42.0f);
}

TEST(TTaskSchedulerTest, OneArgTaskWithValue) {
    TTaskScheduler scheduler;
    auto id = scheduler.add(addOne, 4.0f);
    scheduler.executeAll();
    EXPECT_FLOAT_EQ(scheduler.getResult<float>(id), 5.0f);
}

TEST(TTaskSchedulerTest, OneArgTaskWithFuture) {
    TTaskScheduler scheduler;
    auto id1 = scheduler.add(addOne, 3.0f);
    auto id2 = scheduler.add(addOne, scheduler.getFutureResult<float>(id1));
    scheduler.executeAll();
    EXPECT_FLOAT_EQ(scheduler.getResult<float>(id2), 5.0f);
}

TEST(TTaskSchedulerTest, TwoArgTaskMixedFuture) {
    TTaskScheduler scheduler;
    auto id1 = scheduler.add(addOne, 2.0f);
    auto id2 = scheduler.add(mul, scheduler.getFutureResult<float>(id1), 3.0f);
    scheduler.executeAll();
    EXPECT_FLOAT_EQ(scheduler.getResult<float>(id2), 9.0f);
}

TEST(TTaskSchedulerTest, MethodTaskOneArg) {
    TTaskScheduler scheduler;
    MyClass obj{2.0f};
    auto id = scheduler.add(&MyClass::mul, obj, 5.0f);
    scheduler.executeAll();
    EXPECT_FLOAT_EQ(scheduler.getResult<float>(id), 10.0f);
}

TEST(TTaskSchedulerTest, LambdaNoArgs) {
    TTaskScheduler scheduler;
    auto id = scheduler.add([]() { return 100.0f; });
    scheduler.executeAll();
    EXPECT_FLOAT_EQ(scheduler.getResult<float>(id), 100.0f);
}

TEST(TTaskSchedulerTest, LambdaOneArg) {
    TTaskScheduler scheduler;
    auto id1 = scheduler.add(addOne, 5.0f);
    auto id2 = scheduler.add([](float x) { return x * 10; }, scheduler.getFutureResult<float>(id1));
    scheduler.executeAll();
    EXPECT_FLOAT_EQ(scheduler.getResult<float>(id2), 60.0f);
}

TEST(TTaskSchedulerTest, LambdaTwoArgs) {
    TTaskScheduler scheduler;
    auto id1 = scheduler.add(addOne, 4.0f);
    auto id2 = scheduler.add([](float x, float y) { return x + y; }, scheduler.getFutureResult<float>(id1), 6.0f);
    scheduler.executeAll();
    EXPECT_FLOAT_EQ(scheduler.getResult<float>(id2), 11.0f);
}
