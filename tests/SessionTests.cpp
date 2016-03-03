//
// Created by Tom Fewster on 02/03/2016.
//

#include <gtest/gtest.h>
#include <event/Session.h>
#include <event/IOEvent.h>
#include <event/TimerEvent.h>
#include <event/BusySpinQueue.h>

TEST(Session, Shutdown) {

    EXPECT_EQ(DCF::OK, DCF::Session::initialise());

    const auto startTime = std::chrono::steady_clock::now();
    EXPECT_EQ(DCF::OK, DCF::Session::destroy());
    const auto endTime = std::chrono::steady_clock::now();

    const auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    // Did it happen in a timley manner?
    EXPECT_LT(actual.count(), 50);
}

TEST(Session, SimpleTimeout) {

    EXPECT_EQ(DCF::OK, DCF::Session::initialise());

    bool callbackFired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, pipe(fd));

    DCF::BusySpinQueue queue;

    const auto startTime = std::chrono::steady_clock::now();
    DCF::TimerEvent handler(&queue, std::chrono::milliseconds(10), [&](const DCF::TimerEvent *event) {
        callbackFired = true;
    });

    queue.dispatch();
    const auto endTime = std::chrono::steady_clock::now();
    const auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    EXPECT_TRUE(callbackFired);
    EXPECT_LT(actual.count(), 50);

    EXPECT_EQ(DCF::OK, DCF::Session::destroy());
}

TEST(Session, SimpleRead) {

    EXPECT_EQ(DCF::OK, DCF::Session::initialise());

    bool callbackFired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, pipe(fd));

    DCF::BusySpinQueue queue;

    DCF::IOEvent handler(&queue, fd[0], DCF::EventType::READ, [&](const DCF::IOEvent *event, const DCF::EventType eventType) {
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callbackFired = true;
        char buffer[1];
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
    });

    std::thread signal([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_NE(-1, write(fd[1], "x", 1));
    });
    queue.dispatch(std::chrono::seconds(5));

    signal.join();
    EXPECT_TRUE(callbackFired);

    EXPECT_EQ(DCF::OK, DCF::Session::destroy());
}