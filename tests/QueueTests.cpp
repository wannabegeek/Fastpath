//
// Created by Tom Fewster on 03/03/2016.
//

#include <gtest/gtest.h>
#include <event/Session.h>
#include <event/IOEvent.h>
#include <event/TimerEvent.h>
#include <event/BusySpinQueue.h>
#include <event/BlockingQueue.h>

TEST(BusySpinQueue, Timeout) {

    EXPECT_EQ(DCF::OK, DCF::Session::initialise());
    DCF::BusySpinQueue queue;

    const auto startTime = std::chrono::steady_clock::now();
    queue.dispatch(std::chrono::milliseconds(100));
    const auto endTime = std::chrono::steady_clock::now();
    const auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    EXPECT_GE(actual.count(), 100);
    EXPECT_LT(actual.count(), 200);

    EXPECT_EQ(DCF::OK, DCF::Session::destroy());
}

TEST(BlockingQueue, Timeout) {

    EXPECT_EQ(DCF::OK, DCF::Session::initialise());
    DCF::BlockingQueue queue;

    const auto startTime = std::chrono::steady_clock::now();
    queue.dispatch(std::chrono::milliseconds(100));
    const auto endTime = std::chrono::steady_clock::now();
    const auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    EXPECT_GE(actual.count(), 100);
    EXPECT_LT(actual.count(), 200);

    EXPECT_EQ(DCF::OK, DCF::Session::destroy());
}