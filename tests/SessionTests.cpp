/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include <gtest/gtest.h>
#include <fastpath/event/Session.h>
#include <fastpath/event/IOEvent.h>
#include <fastpath/event/TimerEvent.h>
#include <fastpath/event/BusySpinQueue.h>
#include <fastpath/event/BlockingQueue.h>
#include <fastpath/event/InlineQueue.h>
#include <fastpath/utils/logger.h>

TEST(Session, Shutdown) {

    LOG_LEVEL(tf::logger::info);

    EXPECT_EQ(DCF::OK, DCF::Session::initialise());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    const auto startTime = std::chrono::steady_clock::now();
    EXPECT_EQ(DCF::OK, DCF::Session::destroy());
    const auto endTime = std::chrono::steady_clock::now();

    const auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    // Did it happen in a timley manner?
    EXPECT_GT(50u, actual.count());
}

TEST(Session, SimpleTimeout) {

    EXPECT_EQ(DCF::OK, DCF::Session::initialise());

    bool callbackFired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, pipe(fd));

    DCF::BusySpinQueue queue;

    const auto startTime = std::chrono::steady_clock::now();
    queue.registerEvent(std::chrono::milliseconds(10), [&](const DCF::TimerEvent *event) {
        callbackFired = true;
    });

    EXPECT_EQ(1u, queue.event_count());

    EXPECT_EQ(DCF::OK, queue.dispatch());
    const auto endTime = std::chrono::steady_clock::now();
    const auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    EXPECT_TRUE(callbackFired);
    EXPECT_GT(50u, actual.count());

    EXPECT_EQ(DCF::OK, DCF::Session::destroy());
}

TEST(Session, SimpleReadInline) {

    LOG_LEVEL(tf::logger::info);

    bool callbackFired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, pipe(fd));

    DCF::InlineQueue queue;

    queue.registerEvent(fd[0], DCF::EventType::READ, [&](const DCF::DataEvent *event, const DCF::EventType eventType) {
        DEBUG_LOG("in callback");
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callbackFired = true;
        char buffer[1];
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
    });
    DEBUG_LOG("Registered listener");

    EXPECT_EQ(1u, queue.event_count());

    std::thread signal([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_NE(-1, write(fd[1], "x", 1));
        DEBUG_LOG("Signal thread complete");
    });
    EXPECT_EQ(DCF::OK, queue.dispatch(std::chrono::seconds(5)));

    if (signal.joinable()) {
        signal.join();
    }
    close(fd[0]);
    close(fd[1]);
    EXPECT_TRUE(callbackFired);
}

TEST(Session, SimpleReadBusySpin) {
    LOG_LEVEL(tf::logger::info);
    EXPECT_EQ(DCF::OK, DCF::Session::initialise());

    bool callbackFired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, pipe(fd));

    DCF::BusySpinQueue queue;

    queue.registerEvent(fd[0], DCF::EventType::READ, [&](const DCF::DataEvent *event, const DCF::EventType eventType) {
        DEBUG_LOG("In callback");
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callbackFired = true;
        char buffer[1];
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
    });

    EXPECT_EQ(1u, queue.event_count());

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

    LOG_LEVEL(tf::logger::info);
    queue.registerEvent(fd[0], DCF::EventType::READ, [&](const DCF::DataEvent *event, const DCF::EventType eventType) {
        DEBUG_LOG("In callback");
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callbackFired = true;
        char buffer[1];
        DEBUG_LOG("In read");
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
        DEBUG_LOG("Out read");
        done = true;
    });

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

    EXPECT_EQ(DCF::OK, DCF::Session::initialise());

    bool callbackFired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, pipe(fd));

    DCF::InlineQueue queue;
    bool shutdown = false;

    DCF::DataEvent *handler = nullptr;
    auto callback = [&](DCF::DataEvent *event, const DCF::EventType eventType) {
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callbackFired = true;
        char buffer[1];
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
        shutdown = true;
        queue.unregisterEvent(event);
    };

    unsigned long timerCounter = 0;
    DCF::TimerEvent *timer = queue.registerEvent(std::chrono::milliseconds(100), [&](const DCF::TimerEvent *event) {
        DEBUG_LOG("Still waiting for data");
        if (timerCounter == 0) {
            EXPECT_NE(nullptr, (handler = queue.registerEvent(fd[0], DCF::EventType::READ, callback)));
        }
        timerCounter++;
    });

    std::thread signal([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_NE(-1, write(fd[1], "x", 1));
    });
    while (shutdown == false) {
        queue.dispatch(std::chrono::seconds(5));
    }

    signal.join();
    EXPECT_TRUE(callbackFired);
    queue.unregisterEvent(timer);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
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

    DCF::DataEvent *handler = nullptr;
    auto callback = [&](DCF::DataEvent *event, const DCF::EventType eventType) {
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callbackFired = true;
        char buffer[1];
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
        shutdown = true;
        queue.unregisterEvent(event);
    };

    unsigned long timerCounter = 0;
    queue.registerEvent(std::chrono::milliseconds(100), [&](const DCF::TimerEvent *event) {
        DEBUG_LOG("Still waiting for data");
        if (timerCounter == 0) {
            EXPECT_NE(nullptr, (handler = queue.registerEvent(fd[0], DCF::EventType::READ, callback)));
        }
        timerCounter++;
        if (timerCounter >= 100) {
            shutdown = true;
        }
    });

    std::thread signal([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_NE(-1, write(fd[1], "x", 1));
    });
    while (shutdown == false) {
        queue.dispatch(std::chrono::seconds(5));
    }

    signal.join();
    EXPECT_TRUE(callbackFired);
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

    DCF::DataEvent *handler = nullptr;
    auto callback = [&](DCF::DataEvent *event, const DCF::EventType eventType) {
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callbackFired = true;
        char buffer[1];
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
        shutdown = true;
        queue.unregisterEvent(event);
    };

    unsigned long timerCounter = 0;
    queue.registerEvent(std::chrono::milliseconds(100), [&](const DCF::TimerEvent *event) {
        DEBUG_LOG("Still waiting for data");
        if (timerCounter == 0) {
            EXPECT_NE(nullptr, (handler = queue.registerEvent(fd[0], DCF::EventType::READ, callback)));
        }
        timerCounter++;
        if (timerCounter >= 100) {
            shutdown = true;
        }
    });

    std::thread signal([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_NE(-1, write(fd[1], "x", 1));
    });
    while (shutdown == false) {
        queue.dispatch(std::chrono::seconds(5));
    }

    signal.join();
    EXPECT_TRUE(callbackFired);
    EXPECT_GE(10u, timerCounter);

    EXPECT_EQ(DCF::OK, DCF::Session::destroy());
}

TEST(Session, TimerBacklog) {

    LOG_LEVEL(tf::logger::info);

    EXPECT_EQ(DCF::OK, DCF::Session::initialise());

    bool callbackFired = false;

    DCF::BlockingQueue queue;
    bool shutdown = false;

    queue.registerEvent(std::chrono::milliseconds(10), [&](const DCF::TimerEvent *event) {
        callbackFired = true;
    });

    EXPECT_EQ(1u, queue.event_count());

    std::this_thread::sleep_for(std::chrono::seconds(3));
    EXPECT_LT(200u, queue.eventsInQueue());

    size_t counter = 0;
    while (shutdown == false) {
        queue.dispatch(std::chrono::seconds(5));
        if (counter++ > 1000) {
            break;
        }
    }

    EXPECT_EQ(DCF::OK, DCF::Session::destroy());
}

TEST(Session, TimerUnregisterWithBacklog) {

    LOG_LEVEL(tf::logger::info);

    EXPECT_EQ(DCF::OK, DCF::Session::initialise());

    DCF::BlockingQueue queue;

    std::atomic<int> counter = ATOMIC_VAR_INIT(0);
    /*auto event = */queue.registerEvent(std::chrono::milliseconds(10), [&](DCF::TimerEvent *e) {
        queue.unregisterEvent(e);
        counter++;
        EXPECT_EQ(1, counter);
    });

    EXPECT_EQ(1u, queue.event_count());

    std::this_thread::sleep_for(std::chrono::seconds(3));
    EXPECT_LT(200u, queue.eventsInQueue());

    queue.dispatch(std::chrono::seconds(1));

    EXPECT_EQ(DCF::OK, DCF::Session::destroy());
}
