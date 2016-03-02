#include <gtest/gtest.h>

#include <event/EventManager.h>

#include <chrono>
#include <unistd.h>
#include <event/EventType.h>
#include <event/InlineQueue.h>

TEST(EventManager, SimpleTimeout) {

    DCF::EventManager mgr;

    const auto startTime = std::chrono::steady_clock::now();
    mgr.waitForEvent(std::chrono::milliseconds(100));
    const auto endTime = std::chrono::steady_clock::now();
    const auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    EXPECT_GT(110, actual.count());
    EXPECT_LT(99, actual.count());
}

TEST(EventManager, Notifier) {
    DCF::EventManager mgr;

    std::thread signal([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        mgr.notify();
    });

    const auto startTime = std::chrono::steady_clock::now();
    mgr.waitForEvent(std::chrono::milliseconds(1000));
    const auto endTime = std::chrono::steady_clock::now();

    const auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    EXPECT_LT(actual.count(), 500);

    if (signal.joinable()) {
        signal.join();
    }
}

TEST(EventManager, SimpleRead) {

    DCF::EventManager mgr;
    bool callbackFired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, pipe(fd));

    DCF::InlineQueue queue(mgr);

    DCF::IOEvent handler(&queue, fd[0], DCF::EventType::READ, [&](const DCF::IOEvent *event, const DCF::EventType eventType) {
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callbackFired = true;
        char buffer[1];
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
    });
    mgr.registerHandler(handler);

    std::thread signal([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_NE(-1, write(fd[1], "x", 1));
    });
    queue.dispatch(std::chrono::seconds(5));
    signal.join();
    EXPECT_TRUE(callbackFired);
    mgr.unregisterHandler(handler);
    close(fd[0]);
    close(fd[1]);
}

TEST(EventManager, SimpleTimer) {
    DCF::EventManager mgr;
    bool callbackFired = false;
    DCF::InlineQueue queue(mgr);

    DCF::TimerEvent handler(&queue, std::chrono::seconds(1), [&](const DCF::TimerEvent *event) {
        callbackFired = true;
    });
    mgr.registerHandler(handler);

    mgr.waitForEvent(std::chrono::seconds(3));
    EXPECT_TRUE(callbackFired);
    mgr.unregisterHandler(handler);
}

TEST(EventManager, ResetTimer) {
    DCF::EventManager mgr;
    bool callbackFired = false;
    DCF::InlineQueue queue(mgr);

    DCF::TimerEvent handler(&queue, std::chrono::seconds(1), [&](const DCF::TimerEvent *event) {
        callbackFired = true;
    });
    mgr.registerHandler(handler);

    const auto startTime = std::chrono::steady_clock::now();
    mgr.waitForEvent(std::chrono::milliseconds(500)); //0.5 seconds
    EXPECT_FALSE(callbackFired);
    handler.reset();
    mgr.waitForEvent(std::chrono::seconds(2));
    const auto endTime = std::chrono::steady_clock::now();

    const auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    EXPECT_TRUE(callbackFired);
    EXPECT_LT(actual.count(), 1600);
    EXPECT_GE(actual.count(), 1500);

    mgr.unregisterHandler(handler);
}

TEST(EventManager, ComplexRead) {

    DCF::EventManager mgr;
    bool callback1Fired = false;
    bool callback2Fired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, ::pipe(fd));
    DCF::InlineQueue queue(mgr);

    DCF::IOEvent handler1(&queue, fd[0], DCF::EventType::READ, [&](const DCF::IOEvent *event, const DCF::EventType eventType) {
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callback1Fired = true;
        char buffer[1];
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
    });

    DCF::IOEvent handler2(&queue, fd[0], DCF::EventType::READ, [&](const DCF::IOEvent *event, const DCF::EventType eventType) {
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callback2Fired = true;
        // We can't have both handlers reading the data
    });

    mgr.registerHandler(handler1);
    mgr.registerHandler(handler2);

    std::thread signal([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ASSERT_NE(-1, write(fd[1], "x", 1));
    });
    mgr.waitForEvent(std::chrono::milliseconds(100));
    signal.join();
    EXPECT_TRUE(callback1Fired);
    EXPECT_TRUE(callback2Fired);
    mgr.unregisterHandler(handler1);
    mgr.unregisterHandler(handler2);
    close(fd[0]);
    close(fd[1]);
}

TEST(EventManager, SimpleTimeoutWithActiveFD) {

    DCF::EventManager mgr;
    bool callback1Fired = false;
    bool callback2Fired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, pipe(fd));
    DCF::InlineQueue queue(mgr);

    DCF::TimerEvent timeoutHandler(&queue, std::chrono::milliseconds(10), [&](const DCF::TimerEvent *event) {
       callback1Fired = true;
    });

    DCF::IOEvent pipeHandler(&queue, fd[0], DCF::EventType::READ, [&](const DCF::IOEvent *event, const DCF::EventType eventType) {
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callback2Fired = true;
        char buffer[1];
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
    });
    mgr.registerHandler(timeoutHandler);
    mgr.registerHandler(pipeHandler);

    mgr.waitForEvent(std::chrono::seconds(5));

    EXPECT_TRUE(callback1Fired);
    EXPECT_FALSE(callback2Fired);
    mgr.unregisterHandler(timeoutHandler);
    mgr.unregisterHandler(pipeHandler);
    close(fd[0]);
    close(fd[1]);
}
