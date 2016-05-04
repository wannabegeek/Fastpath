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

#include "fastpath/event/EventManager.h"

#include <chrono>
#include <unistd.h>
#include "fastpath/event/EventType.h"
#include "fastpath/event/IOEvent.h"
#include "fastpath/event/TimerEvent.h"
#include "fastpath/event/InlineQueue.h"

#include "fastpath/utils/logger.h"

//TEST(EventManager, SimpleTimeout) {
//
//    fp::InlineEventManager mgr;
//
//    const auto startTime = std::chrono::steady_clock::now();
//    mgr.waitForEvent(std::chrono::milliseconds(100));
//    const auto endTime = std::chrono::steady_clock::now();
//    const auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
//    EXPECT_GE(110, actual.count());
//    EXPECT_LT(99, actual.count());
//}

//TEST(EventManager, Notifier) {
//    // Notifier isn't valid for inline
//    fp::GlobalEventManager mgr;
//
//    std::thread signal([&]() {
//        std::this_thread::sleep_for(std::chrono::milliseconds(10));
//        mgr.notify();
//    });
//
//    const auto startTime = std::chrono::steady_clock::now();
//    mgr.waitForEvent(std::chrono::milliseconds(1000));
//    const auto endTime = std::chrono::steady_clock::now();
//
//    const auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
//    EXPECT_LT(actual.count(), 500);
//
//    if (signal.joinable()) {
//        signal.join();
//    }
//}

TEST(EventManager, SimpleRead) {

    LOG_LEVEL(tf::logger::info);

    bool callbackFired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, pipe(fd));

    fp::InlineQueue queue;

    queue.registerEvent(fd[0], fp::EventType::READ, [&](const fp::DataEvent *event, const fp::EventType eventType) {
        EXPECT_EQ(fp::EventType::READ, eventType);
        callbackFired = true;
        char buffer[1];
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
    });

    std::thread signal([&]() {
        DEBUG_LOG("sleeping");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        DEBUG_LOG("Signalling to other thread");
        ASSERT_NE(-1, write(fd[1], "x", 1));
    });
    DEBUG_LOG("Dispatching");
    EXPECT_NE(fp::EVM_NOTRUNNING, queue.dispatch(std::chrono::seconds(5)));
    DEBUG_LOG("...complete, waiting for thread to exit");
    signal.join();
    EXPECT_TRUE(callbackFired);
    close(fd[0]);
    close(fd[1]);
}

TEST(EventManager, SimpleTimer) {
    bool callbackFired = false;
    fp::InlineQueue queue;

    queue.registerEvent(std::chrono::seconds(1), [&](const fp::TimerEvent *event) {
        callbackFired = true;
    });

    queue.dispatch(std::chrono::seconds(3));
    EXPECT_TRUE(callbackFired);
}

TEST(EventManager, ResetTimer) {
    bool callbackFired = false;
    fp::InlineQueue queue;

    fp::TimerEvent *timer = queue.registerEvent(std::chrono::seconds(1), [&](const fp::TimerEvent *event) {
        callbackFired = true;
    });

    const auto startTime = std::chrono::steady_clock::now();
    queue.dispatch(std::chrono::milliseconds(500)); //0.5 seconds
    EXPECT_FALSE(callbackFired);
    timer->reset();
    queue.dispatch(std::chrono::seconds(2));
    const auto endTime = std::chrono::steady_clock::now();

    const auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    EXPECT_TRUE(callbackFired);
    EXPECT_LT(actual.count(), 1600);
    EXPECT_GE(actual.count(), 1500);
}

TEST(EventManager, ComplexRead) {

    bool callback1Fired = false;
    bool callback2Fired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, ::pipe(fd));
    fp::InlineQueue queue;

    queue.registerEvent(fd[0], fp::EventType::READ, [&](const fp::DataEvent *event, const fp::EventType eventType) {
        EXPECT_EQ(fp::EventType::READ, eventType);
        callback1Fired = true;
        char buffer[1];
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
    });

    queue.registerEvent(fd[0], fp::EventType::READ, [&](const fp::DataEvent *event, const fp::EventType eventType) {
        EXPECT_EQ(fp::EventType::READ, eventType);
        callback2Fired = true;
        // We can't have both handlers reading the data
    });

    std::thread signal([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ASSERT_NE(-1, write(fd[1], "x", 1));
    });
    queue.dispatch(std::chrono::milliseconds(100));
    signal.join();
    EXPECT_TRUE(callback1Fired);
    EXPECT_TRUE(callback2Fired);
    close(fd[0]);
    close(fd[1]);
}

TEST(EventManager, SimpleTimeoutWithActiveFD) {

    bool callback1Fired = false;
    bool callback2Fired = false;
    int fd[2] = {0, 0};
    ASSERT_NE(-1, pipe(fd));
    fp::InlineQueue queue;

    queue.registerEvent(std::chrono::milliseconds(10), [&](const fp::TimerEvent *event) {
       callback1Fired = true;
    });

    queue.registerEvent(fd[0], fp::EventType::READ, [&](const fp::DataEvent *event, const fp::EventType eventType) {
        EXPECT_EQ(fp::EventType::READ, eventType);
        callback2Fired = true;
        char buffer[1];
        EXPECT_NE(-1, read(fd[0], &buffer, 1));
    });

    queue.dispatch(std::chrono::seconds(5));

    EXPECT_TRUE(callback1Fired);
    EXPECT_FALSE(callback2Fired);
    close(fd[0]);
    close(fd[1]);
}
