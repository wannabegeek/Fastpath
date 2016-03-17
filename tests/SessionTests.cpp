//
// Created by Tom Fewster on 02/03/2016.
//

#include <gtest/gtest.h>
#include <event/Session.h>
#include <event/IOEvent.h>
#include <event/TimerEvent.h>
#include <event/BusySpinQueue.h>
#include <event/BlockingQueue.h>
#include <event/InlineQueue.h>
#include <utils/logger.h>

TEST(Session, Shutdown) {

    LOG_LEVEL(tf::logger::debug);

    EXPECT_EQ(DCF::OK, DCF::Session::initialise());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

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
    DCF::TimerEvent handler(std::chrono::milliseconds(10), [&](const DCF::TimerEvent *event) {
        callbackFired = true;
    });

    EXPECT_EQ(DCF::OK, queue.registerEvent(handler));
    EXPECT_EQ(DCF::OK, queue.dispatch());
    const auto endTime = std::chrono::steady_clock::now();
    const auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    EXPECT_TRUE(callbackFired);
    EXPECT_LT(actual.count(), 50);

    EXPECT_EQ(DCF::OK, DCF::Session::destroy());
}

TEST(Session, SimpleReadInline) {

    EXPECT_EQ(DCF::OK, DCF::Session::initialise());

    bool callbackFired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, pipe(fd));

    DCF::InlineQueue queue;

    DCF::IOEvent handler(fd[0], DCF::EventType::READ, [&](const DCF::IOEvent *event, const DCF::EventType eventType) {
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callbackFired = true;
        char buffer[1];
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
    });
    EXPECT_EQ(DCF::OK, queue.registerEvent(handler));

    std::thread signal([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_NE(-1, write(fd[1], "x", 1));
    });
    EXPECT_EQ(DCF::OK, queue.dispatch(std::chrono::seconds(5)));

    signal.join();
    EXPECT_TRUE(callbackFired);

    EXPECT_EQ(DCF::OK, DCF::Session::destroy());
}

TEST(Session, SimpleReadBusySpin) {

    EXPECT_EQ(DCF::OK, DCF::Session::initialise());

    bool callbackFired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, pipe(fd));

    DCF::BusySpinQueue queue;

    LOG_LEVEL(tf::logger::debug);
    DCF::IOEvent handler(fd[0], DCF::EventType::READ, [&](const DCF::IOEvent *event, const DCF::EventType eventType) {
        DEBUG_LOG("In callback");
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callbackFired = true;
        char buffer[1];
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
    });

    EXPECT_EQ(DCF::OK, queue.registerEvent(handler));
    // we need to make sure wwe have registered with the event loop
    while(!handler.isRegistered());

    std::thread signal([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_NE(-1, write(fd[1], "x", 1));
        DEBUG_LOG("Written to pipe");
    });
    DEBUG_LOG("Dispatching...");
    EXPECT_EQ(DCF::OK, queue.dispatch(std::chrono::seconds(5)));
    DEBUG_LOG("done disptach");

    signal.join();
    EXPECT_TRUE(callbackFired);

    EXPECT_EQ(DCF::OK, DCF::Session::destroy());
}

TEST(Session, SimpleReadBlocking) {

    EXPECT_EQ(DCF::OK, DCF::Session::initialise());

    bool callbackFired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, pipe(fd));
    bool done = false;

    DCF::BlockingQueue queue;

    LOG_LEVEL(tf::logger::debug);
    DCF::IOEvent handler(fd[0], DCF::EventType::READ, [&](const DCF::IOEvent *event, const DCF::EventType eventType) {
        DEBUG_LOG("In callback");
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callbackFired = true;
        char buffer[1];
        DEBUG_LOG("In read");
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
        DEBUG_LOG("Out read");
        done = true;
    });
    EXPECT_EQ(DCF::OK, queue.registerEvent(handler));

    std::thread signal([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_NE(-1, write(fd[1], "x", 1));
        DEBUG_LOG("Written to pipe");
    });
    DEBUG_LOG("Dispatching...");
    while (!done) {
        queue.dispatch(std::chrono::seconds(5));
    }
    DEBUG_LOG("done disptach");

    signal.join();
    EXPECT_TRUE(callbackFired);

    EXPECT_EQ(DCF::OK, DCF::Session::destroy());
}


TEST(Session, ReadTimerInline) {

    LOG_LEVEL(tf::logger::debug);

    EXPECT_EQ(DCF::OK, DCF::Session::initialise());

    bool callbackFired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, pipe(fd));

    DCF::InlineQueue queue;
    bool shutdown = false;

    DCF::IOEvent handler;
    auto callback = [&](DCF::IOEvent *event, const DCF::EventType eventType) {
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callbackFired = true;
        char buffer[1];
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
        shutdown = true;
        event->destroy();
    };

    unsigned long timerCounter = 0;
    DCF::TimerEvent timer(std::chrono::milliseconds(100), [&](const DCF::TimerEvent *event) {
        INFO_LOG("Still waiting for data");
        if (timerCounter == 0) {
            ASSERT_FALSE(handler.isRegistered());
            EXPECT_EQ(DCF::OK, handler.create(fd[0], DCF::EventType::READ, callback));
            EXPECT_EQ(DCF::OK, queue.registerEvent(handler));
        }
        EXPECT_TRUE(handler.isRegistered());
        timerCounter++;
    });
    EXPECT_EQ(DCF::OK, queue.registerEvent(timer));

    std::thread signal([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_NE(-1, write(fd[1], "x", 1));
    });
    while (shutdown == false) {
        queue.dispatch(std::chrono::seconds(5));
    }

    signal.join();
    EXPECT_TRUE(callbackFired);
    timer.destroy();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    EXPECT_FALSE(handler.isRegistered());
    EXPECT_GE(10u, timerCounter);

    EXPECT_EQ(DCF::OK, DCF::Session::destroy());
}

TEST(Session, ReadTimerBusySpin) {

    EXPECT_EQ(DCF::OK, DCF::Session::initialise());

    bool callbackFired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, pipe(fd));

    DCF::BusySpinQueue queue;

    bool shutdown = false;

    DCF::IOEvent handler;
    auto callback = [&](DCF::IOEvent *event, const DCF::EventType eventType) {
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callbackFired = true;
        char buffer[1];
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
        shutdown = true;
        event->destroy();
    };

    unsigned long timerCounter = 0;
    DCF::TimerEvent timer(std::chrono::milliseconds(100), [&](const DCF::TimerEvent *event) {
        INFO_LOG("Still waiting for data");
        if (timerCounter == 0) {
            ASSERT_FALSE(handler.isRegistered());
            EXPECT_EQ(DCF::OK, handler.create(fd[0], DCF::EventType::READ, callback));
            EXPECT_EQ(DCF::OK, queue.registerEvent(handler));
        }
        timerCounter++;
    });
    EXPECT_EQ(DCF::OK, queue.registerEvent(timer));

    std::thread signal([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_NE(-1, write(fd[1], "x", 1));
    });
    while (shutdown == false) {
        queue.dispatch(std::chrono::seconds(5));
    }

    signal.join();
    EXPECT_TRUE(callbackFired);
    EXPECT_FALSE(handler.isRegistered());
    EXPECT_GE(10u, timerCounter);

    EXPECT_EQ(DCF::OK, DCF::Session::destroy());
}

TEST(Session, ReadTimerBlocking) {

    EXPECT_EQ(DCF::OK, DCF::Session::initialise());

    bool callbackFired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, pipe(fd));

    DCF::BlockingQueue queue;
    bool shutdown = false;

    DCF::IOEvent handler;
    auto callback = [&](DCF::IOEvent *event, const DCF::EventType eventType) {
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callbackFired = true;
        char buffer[1];
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
        shutdown = true;
        event->destroy();
    };

    unsigned long timerCounter = 0;
    DCF::TimerEvent timer(std::chrono::milliseconds(100), [&](const DCF::TimerEvent *event) {
        INFO_LOG("Still waiting for data");
        if (timerCounter == 0) {
            ASSERT_FALSE(handler.isRegistered());
            EXPECT_EQ(DCF::OK, handler.create(fd[0], DCF::EventType::READ, callback));
            EXPECT_EQ(DCF::OK, queue.registerEvent(handler));
        }
        EXPECT_TRUE(handler.isRegistered());
        timerCounter++;
    });
    EXPECT_EQ(DCF::OK, queue.registerEvent(timer));

    std::thread signal([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_NE(-1, write(fd[1], "x", 1));
    });
    while (shutdown == false) {
        queue.dispatch(std::chrono::seconds(5));
    }

    signal.join();
    EXPECT_TRUE(callbackFired);
    EXPECT_FALSE(handler.isRegistered());
    EXPECT_GE(10u, timerCounter);

    EXPECT_EQ(DCF::OK, DCF::Session::destroy());
}
