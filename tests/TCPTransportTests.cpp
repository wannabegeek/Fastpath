//
// Created by Tom Fewster on 04/03/2016.
//

#include <gtest/gtest.h>
#include <transport/TCPTransport.h>
#include <thread>
#include <memory>
#include <utils/logger.h>
#include <transport/TCPSocketServer.h>
#include <event/InlineQueue.h>
#include <event/BlockingQueue.h>
#include <event/IOEvent.h>
#include <event/EventManager.h>
#include <messages/Message.h>
//#include <utils/ByteStorage.h>


TEST(TCPTransport, TryConnectFail) {
    LOG_LEVEL(tf::logger::info);

    DCF::TCPTransport transport("dcf://localhost:1234", "unit test");
    std::this_thread::sleep_for(std::chrono::seconds(15));
    EXPECT_FALSE(transport.valid());

    DCF::Message msg;
    msg.addDataField("Name", "Tom Fewster");
    EXPECT_EQ(DCF::CANNOT_SEND, transport.sendMessage(msg));
}

TEST(TCPTransport, TryConnectSuccess) {
    static const int MTU = 1500;

    LOG_LEVEL(tf::logger::info);

    DCF::TCPTransport transport("dcf://localhost:6867", "unit test");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(transport.valid());

    DCF::Message sendMsg;
    sendMsg.setSubject("UNIT.TEST");
    sendMsg.addDataField("Name", "Tom Fewster");

    std::thread server([&]() {

        DCF::TCPSocketServer svr("localhost", "6867");
        ASSERT_TRUE(svr.connect(DCF::SocketOptionsNonBlocking));
        ASSERT_NE(-1, svr.getSocket());

        DEBUG_LOG("Thread waiting for connections");

        DCF::InlineEventManager serverMgr;
        std::unique_ptr<DCF::Socket> connection;
        bool finished = false;

        DCF::InlineQueue queue;
        DCF::DataEvent *clientHandler = nullptr;

        auto client = [&](DCF::DataEvent *event, int eventType) {
            DEBUG_LOG("In client callback");
            EXPECT_EQ(DCF::EventType::READ, eventType);
            char buffer[MTU];
            int counter = 0;
            ssize_t size = 0;
            while (true) {
                DCF::Socket::ReadResult result = connection->read(buffer, MTU, size); // 1500 is the typical MTU size
                DEBUG_LOG("got stuff");
                if (result == DCF::Socket::MoreData) {
                    DCF::Message msg;
                    EXPECT_TRUE(msg.decode(DCF::MessageBuffer::ByteStorageType(reinterpret_cast<const byte *>(buffer), size, true)));
                    DEBUG_LOG("Received " << msg);
                    EXPECT_EQ(sendMsg, msg);
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
            DEBUG_LOG("Callback completed");
            queue.unregisterEvent(event);
        };

        queue.registerEvent(svr.getSocket(), DCF::EventType::READ, [&](const DCF::DataEvent *event, int eventType) {
            EXPECT_EQ(DCF::EventType::READ, eventType);
            DEBUG_LOG("entering accept");
            connection = svr.acceptPendingConnection();
            if (connection != nullptr) {
                DEBUG_LOG("out of accept");

                clientHandler = queue.registerEvent(connection->getSocket(), DCF::EventType::READ, client);
                DEBUG_LOG("registered new client on socket: " << connection->getSocket());
            }
        });

        DEBUG_LOG("Ready to dispatch");
        while(!finished) {
            queue.dispatch(std::chrono::seconds(3));
            DEBUG_LOG("Dispatch an event");
        }

        connection->disconnect();
        DEBUG_LOG("Exiting thread");
    });

    std::this_thread::sleep_for(std::chrono::seconds(3));
    EXPECT_TRUE(transport.valid());

    DEBUG_LOG("Sending Message");
    EXPECT_EQ(DCF::OK, transport.sendMessage(sendMsg));
    DEBUG_LOG("..sent");
    server.join();
}

TEST(TCPTransport, TryConnectSuccessFragmented) {
    static const int MTU = 15;

    LOG_LEVEL(tf::logger::info);

    DCF::TCPTransport transport("dcf://localhost:6867", "unit test");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(transport.valid());

    DCF::Message sendMsg;
    sendMsg.setSubject("UNIT.TEST");
    sendMsg.addDataField("Name", "Tom Fewster");

    std::thread server([&]() {

        DCF::TCPSocketServer svr("localhost", "6867");
        ASSERT_TRUE(svr.connect(DCF::SocketOptionsNonBlocking));
        ASSERT_NE(-1, svr.getSocket());

        DEBUG_LOG("Thread waiting for connections");

        DCF::InlineEventManager serverMgr;
        std::unique_ptr<DCF::Socket> connection;
        bool finished = false;

        DCF::InlineQueue queue;
        DCF::DataEvent *clientHandler = nullptr;

        bool consumedMessage = false;
        DCF::MessageBuffer msgBuffer(1025);

        auto client = [&](DCF::DataEvent *event, int eventType) {
            DEBUG_LOG("In client callback");
            EXPECT_EQ(DCF::EventType::READ, eventType);
            int counter = 0;
            ssize_t size = 0;
            while (true) {
                byte *buffer = msgBuffer.allocate(MTU);
                DCF::Socket::ReadResult result = connection->read(reinterpret_cast<char *>(buffer), MTU, size); // 1500 is the typical MTU size
                msgBuffer.erase_back(MTU - size);
                DEBUG_LOG("got stuff");
                if (result == DCF::Socket::MoreData) {
                    DCF::Message msg;
                    DEBUG_LOG("So far we have received " << msgBuffer.length() << " bytes");
                    if (msg.decode(msgBuffer.byteStorage())) {
                        DEBUG_LOG("Received " << msg);
                        EXPECT_EQ(sendMsg, msg);
                        consumedMessage = true;
                        finished = true;
                        queue.unregisterEvent(event);
                        break;
                    }
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
        };

        queue.registerEvent(svr.getSocket(), DCF::EventType::READ, [&](const DCF::DataEvent *event, int eventType) {
            EXPECT_EQ(DCF::EventType::READ, eventType);
            DEBUG_LOG("entering accept");
            connection = svr.acceptPendingConnection();
            if (connection != nullptr) {
                DEBUG_LOG("out of accept");

                clientHandler = queue.registerEvent(connection->getSocket(), DCF::EventType::READ, client);
                DEBUG_LOG("registered new client on socket: " << connection->getSocket());
            }
        });

        DEBUG_LOG("Ready to dispatch");
        while(!finished) {
            queue.dispatch(std::chrono::seconds(3));
            DEBUG_LOG("Dispatch an event");
        }

        connection->disconnect();
        DEBUG_LOG("Exiting thread");
    });

    std::this_thread::sleep_for(std::chrono::seconds(3));
    EXPECT_TRUE(transport.valid());

    DEBUG_LOG("Sending Message");
    EXPECT_EQ(DCF::OK, transport.sendMessage(sendMsg));
    DEBUG_LOG("..sent");
    server.join();
}