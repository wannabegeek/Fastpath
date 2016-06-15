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
#include "fastpath/event/Session.h"
#include "fastpath/event/IOEvent.h"
#include "fastpath/event/TimerEvent.h"
#include "fastpath/event/BusySpinQueue.h"
#include "fastpath/event/BlockingQueue.h"
#include "fastpath/event/InlineQueue.h"

TEST(BusySpinQueue, Timeout) {

    EXPECT_EQ(fp::OK, fp::Session::initialise());
    fp::BusySpinQueue queue;

    const auto startTime = std::chrono::steady_clock::now();
    queue.dispatch(std::chrono::milliseconds(100));
    const auto endTime = std::chrono::steady_clock::now();
    const auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    EXPECT_GE(actual.count(), 100);
    EXPECT_LT(actual.count(), 200);

    EXPECT_EQ(fp::OK, fp::Session::destroy());
}

TEST(BlockingQueue, Timeout) {

    EXPECT_EQ(fp::OK, fp::Session::initialise());
    fp::BlockingQueue queue;

    const auto startTime = std::chrono::steady_clock::now();
    queue.dispatch(std::chrono::milliseconds(100));
    const auto endTime = std::chrono::steady_clock::now();
    const auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    EXPECT_GE(actual.count(), 100);
    EXPECT_LT(actual.count(), 200);

    EXPECT_EQ(fp::OK, queue.dispatch(std::chrono::seconds(5)));

    EXPECT_EQ(fp::OK, fp::Session::destroy());
}

TEST(InlineQueue, RemoveEventInCallback) {

    LOG_LEVEL(tf::logger::debug);

    fp::InlineQueue queue;

    fp::TimerEvent *evt = nullptr;
    evt = queue.registerEvent(std::chrono::milliseconds(100), [&](const fp::TimerEvent *event) {
        DEBUG_LOG("*** in callback ***");
        queue.unregisterEvent(evt);
    });

    EXPECT_EQ(fp::OK, queue.dispatch());
}