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

#include <fastpath/event/EventManager.h>
#include <fastpath/event/IOEvent.h>
#include <fastpath/event/InlineQueue.h>
#include <fastpath/transport/TCPSocketServer.h>
#include <fastpath/transport/TCPSocketClient.h>

#include <chrono>
#include <memory>
#include <thread>
#include <unistd.h>
#include <fastpath/utils/logger.h>
#include <fastpath/event/BusySpinQueue.h>

TEST(Socket, SimpleReadWrite) {

    std::thread server([&]() {
        DCF::TCPSocketServer svr("localhost", "6969");
        ASSERT_TRUE(svr.connect());
        std::shared_ptr<DCF::Socket> connection = svr.acceptPendingConnection();

        char buffer[16];
        int counter = 0;
        ssize_t size = 0;
        while(true) {
            DCF::Socket::ReadResult result = connection->read(buffer, 16, size);
            if (result == DCF::Socket::MoreData) {
                EXPECT_EQ(4, size);
                EXPECT_STREQ(buffer, "tom");
                break;
            } else if (result == DCF::Socket::NoData) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
                if (counter++ > 1000) {
                    break;
                }
            } else if (result == DCF::Socket::Closed) {
                DEBUG_LOG("Socket closed");
                break;
            }
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));
    DCF::TCPSocketClient client("localhost", "6969");
    ASSERT_TRUE(client.connect());
    EXPECT_TRUE(client.send("tom", 4)); // send 4bytes (including \0)

    server.join();
}

TEST(Socket, NonBlockingReadWrite) {
    bool callbackFired = false;

    LOG_LEVEL(tf::logger::info);

    DCF::Session::initialise();

    std::thread server([&]() {
        DEBUG_LOG("Started server thread");
        DCF::TCPSocketServer svr("localhost", "6967");
        ASSERT_TRUE(svr.connect());
        std::shared_ptr<DCF::Socket> connection = svr.acceptPendingConnection();
        DEBUG_LOG("Accepted connection");

        char buffer[16];
        int counter = 0;
        ssize_t size = 0;
        while(true) {
            DCF::Socket::ReadResult result = connection->read(buffer, 16, size);
            if (result == DCF::Socket::MoreData) {
                DEBUG_LOG("Server received data");
                EXPECT_EQ(6, size);
                EXPECT_STREQ(buffer, "hello");
                break;
            } else if (result == DCF::Socket::NoData) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
                if (counter++ > 1000) {
                    break;
                }
            } else if (result == DCF::Socket::Closed) {
                DEBUG_LOG("Socket closed");
                break;
            }
        }

        DEBUG_LOG("Server sending data");
        EXPECT_TRUE(connection->send("goodbye", 8));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    DCF::TCPSocketClient client("localhost", "6967");
    DCF::BusySpinQueue queue;


    DEBUG_LOG("Client attempting to connect");
    ASSERT_TRUE(client.connect(DCF::SocketOptionsDisableSigPipe));
    EXPECT_NE(-1, client.getSocket());
    DEBUG_LOG("Client connected");

    /*DCF::IOEvent *handler = */queue.registerEvent(client.getSocket(), DCF::EventType::READ, [&](const DCF::DataEvent *event, DCF::EventType eventType) {
        EXPECT_EQ(DCF::EventType::READ, eventType);
        DEBUG_LOG("Client received data");
        callbackFired = true;

        int counter = 0;
        char buffer[16];
        ssize_t size = 0;
        while(true) {
            DCF::Socket::ReadResult result = client.read(buffer, 16, size);
            if (result == DCF::Socket::MoreData) {
                EXPECT_EQ(8, size);
                EXPECT_STREQ(buffer, "goodbye");
                break;
            } else if (result == DCF::Socket::NoData) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
                if (counter++ > 1000) {
                    break;
                }
            } else if (result == DCF::Socket::Closed) {
                DEBUG_LOG("Socket closed");
                break;
            }
        }
    });

    DEBUG_LOG("Client sending data");
    EXPECT_TRUE(client.send("hello", 6));

    queue.dispatch(std::chrono::seconds(5));
    EXPECT_TRUE(callbackFired);
    server.join();

    DCF::Session::destroy();
}

TEST(Socket, NonBlockingServerReadWrite) {
    bool callbackFired = false;

    LOG_LEVEL(tf::logger::info);
    DCF::Session::initialise();

    std::thread server([&]() {
        DCF::TCPSocketServer svr("localhost", "6966");
        ASSERT_TRUE(svr.connect(DCF::SocketOptionsNonBlocking));
        ASSERT_NE(-1, svr.getSocket());

        std::shared_ptr<DCF::Socket> connection;
        bool finished = false;

        DCF::BusySpinQueue queue;

        auto client = [&](const DCF::DataEvent *event, int eventType) {
            EXPECT_EQ(DCF::EventType::READ, eventType);
            char buffer[16];
            int counter = 0;
            ssize_t size = 0;
            while (true) {
                DCF::Socket::ReadResult result = connection->read(buffer, 16, size);
                DEBUG_LOG("got stuff");
                if (result == DCF::Socket::MoreData) {
                    EXPECT_EQ(6, size);
                    EXPECT_STREQ(buffer, "hello");
                    break;
                } else if (result == DCF::Socket::NoData) {
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                    if (counter++ > 1000) {
                        break;
                    }
                } else if (result == DCF::Socket::Closed) {
                    DEBUG_LOG("Server Socket closed");
                    break;
                }
            }

            EXPECT_TRUE(connection->send("goodbye", 8));
            finished = true;
        };

        DCF::DataEvent *clientHandler = nullptr;

        /*DCF::DataEvent *handler = */queue.registerEvent(svr.getSocket(), DCF::EventType::READ, [&](const DCF::DataEvent *event, int eventType) {
            EXPECT_EQ(DCF::EventType::READ, eventType);
            DEBUG_LOG("entering accept");
            connection = svr.acceptPendingConnection();
            if (connection != nullptr) {
                DEBUG_LOG("out of accept");

                clientHandler = queue.registerEvent(connection->getSocket(), DCF::EventType::READ, client);

                DEBUG_LOG("registered new client");
            }
        });

        for (int i = 0; !finished && i < 15; i++) {
            DEBUG_LOG("wait...");
            queue.dispatch(std::chrono::milliseconds(1000));
            DEBUG_LOG("...serviced event");
        }
    });

    DCF::BusySpinQueue queue;

    std::this_thread::sleep_for(std::chrono::microseconds(100));
    DCF::TCPSocketClient client("localhost", "6966");

    ASSERT_TRUE(client.connect(DCF::SocketOptionsDisableSigPipe));
    EXPECT_NE(-1, client.getSocket());

    /*DCF::DataEvent *handler = */queue.registerEvent(client.getSocket(), DCF::EventType::READ, [&](const DCF::DataEvent *event, int eventType) {
        EXPECT_EQ(DCF::EventType::READ, eventType);
        callbackFired = true;

        int counter = 0;
        char buffer[16];
        ssize_t size = 0;
        while(true) {
            DCF::Socket::ReadResult result = client.read(buffer, 16, size);
            if (result == DCF::Socket::MoreData) {
                EXPECT_EQ(8, size);
                EXPECT_STREQ(buffer, "goodbye");
                break;
            } else if (result == DCF::Socket::NoData) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
                if (counter++ > 1000) {
                    break;
                }
            } else if (result == DCF::Socket::Closed) {
                DEBUG_LOG("Client Socket closed");
                break;
            }
        }
    });

    EXPECT_TRUE(client.send("hello", 6));

    queue.dispatch(std::chrono::seconds(5));
    EXPECT_TRUE(callbackFired);

    server.join();
    DCF::Session::destroy();
}
