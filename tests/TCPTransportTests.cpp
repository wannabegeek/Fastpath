//
// Created by Tom Fewster on 04/03/2016.
//

#include <gtest/gtest.h>
#include <fastpath/transport/TCPTransport.h>
#include <thread>
#include <memory>
#include <fastpath/utils/logger.h>
#include <fastpath/transport/socket/TCPSocketServer.h>
#include <fastpath/event/InlineQueue.h>
#include <fastpath/event/BlockingQueue.h>
#include <fastpath/event/IOEvent.h>
#include <fastpath/event/EventManager.h>
#include <fastpath/messages/MutableMessage.h>
#include <fastpath/messages/MessageCodec.h>
//#include <utils/ByteStorage.h>


TEST(TCPTransport, TryConnectFail) {
    LOG_LEVEL(tf::logger::info);

    fp::TCPTransport transport("dcf://localhost:1234", "unit test");
    std::this_thread::sleep_for(std::chrono::seconds(15));
    EXPECT_FALSE(transport.valid());

    fp::MutableMessage msg;
    msg.addDataField("Name", "Tom Fewster");
    EXPECT_EQ(fp::CANNOT_SEND, transport.sendMessage(msg));
}

TEST(TCPTransport, TryConnectSuccess) {
    static const int MTU = 1500;

    LOG_LEVEL(tf::logger::info);

    fp::TCPTransport transport("dcf://localhost:6867", "unit test");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(transport.valid());

    fp::MutableMessage sendMsg;
    sendMsg.setSubject("UNIT.TEST");
    sendMsg.addDataField("Name", "Tom Fewster");

    std::thread server([&]() {

        fp::TCPSocketServer svr("localhost", "6867");
        ASSERT_TRUE(svr.connect(fp::SocketOptionsNonBlocking));
        ASSERT_NE(-1, svr.getSocket());

        DEBUG_LOG("Thread waiting for connections");

        fp::InlineEventManager serverMgr;
        std::unique_ptr<fp::Socket> connection;
        bool finished = false;

        fp::InlineQueue queue;
        fp::DataEvent *clientHandler = nullptr;

        auto client = [&](fp::DataEvent *event, int eventType) {
            DEBUG_LOG("In client callback");
            EXPECT_EQ(fp::EventType::READ, eventType);
            char buffer[MTU];
            int counter = 0;
            ssize_t size = 0;
            while (true) {
                fp::Socket::ReadResult result = connection->read(buffer, MTU, size); // 1500 is the typical MTU size
                DEBUG_LOG("got stuff");
                if (result == fp::Socket::MoreData) {
                    fp::MutableMessage msg;
                    EXPECT_TRUE(fp::MessageCodec::decode(&msg, fp::MessageBuffer::ByteStorageType(reinterpret_cast<const byte *>(buffer), size, true)));
                    DEBUG_LOG("Received " << msg);
                    EXPECT_EQ(sendMsg, msg);
                    break;
                } else if (result == fp::Socket::NoData) {
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                    if (counter++ > 1000) {
                        break;
                    }
                } else if (result == fp::Socket::Closed) {
                    DEBUG_LOG("Server Socket closed");
                    break;
                }
            }

            EXPECT_TRUE(connection->send("goodbye", 8));
            finished = true;
            DEBUG_LOG("Callback completed");
            queue.unregisterEvent(event);
        };

        queue.registerEvent(svr.getSocket(), fp::EventType::READ, [&](const fp::DataEvent *event, int eventType) {
            EXPECT_EQ(fp::EventType::READ, eventType);
            DEBUG_LOG("entering accept");
            connection = svr.acceptPendingConnection();
            if (connection != nullptr) {
                DEBUG_LOG("out of accept");

                clientHandler = queue.registerEvent(connection->getSocket(), fp::EventType::READ, client);
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
    EXPECT_EQ(fp::OK, transport.sendMessage(sendMsg));
    DEBUG_LOG("..sent");
    server.join();
}

TEST(TCPTransport, TryConnectSuccessFragmented) {
    static const int MTU = 15;

    LOG_LEVEL(tf::logger::info);

    fp::TCPTransport transport("dcf://localhost:6867", "unit test");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(transport.valid());

    fp::MutableMessage sendMsg;
    sendMsg.setSubject("UNIT.TEST");
    sendMsg.addDataField("Name", "Tom Fewster");

    std::thread server([&]() {

        fp::TCPSocketServer svr("localhost", "6867");
        ASSERT_TRUE(svr.connect(fp::SocketOptionsNonBlocking));
        ASSERT_NE(-1, svr.getSocket());

        DEBUG_LOG("Thread waiting for connections");

        fp::InlineEventManager serverMgr;
        std::unique_ptr<fp::Socket> connection;
        bool finished = false;

        fp::InlineQueue queue;
        fp::DataEvent *clientHandler = nullptr;

        bool consumedMessage = false;
        fp::MessageBuffer msgBuffer(1025);

        auto client = [&](fp::DataEvent *event, int eventType) {
            DEBUG_LOG("In client callback");
            EXPECT_EQ(fp::EventType::READ, eventType);
            int counter = 0;
            ssize_t size = 0;
            while (true) {
                byte *buffer = msgBuffer.allocate(MTU);
                fp::Socket::ReadResult result = connection->read(reinterpret_cast<char *>(buffer), MTU, size); // 1500 is the typical MTU size
                msgBuffer.erase_back(MTU - size);
                DEBUG_LOG("got stuff");
                if (result == fp::Socket::MoreData) {
                    fp::Message msg;
                    DEBUG_LOG("So far we have received " << msgBuffer.length() << " bytes");
                    if (fp::MessageCodec::decode(&msg, msgBuffer.byteStorage())) {
                        DEBUG_LOG("Received " << msg);
                        EXPECT_EQ(sendMsg, msg);
                        consumedMessage = true;
                        finished = true;
                        queue.unregisterEvent(event);
                        break;
                    }
                } else if (result == fp::Socket::NoData) {
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                    if (counter++ > 1000) {
                        break;
                    }
                } else if (result == fp::Socket::Closed) {
                    DEBUG_LOG("Server Socket closed");
                    break;
                }
            }
        };

        queue.registerEvent(svr.getSocket(), fp::EventType::READ, [&](const fp::DataEvent *event, int eventType) {
            EXPECT_EQ(fp::EventType::READ, eventType);
            DEBUG_LOG("entering accept");
            connection = svr.acceptPendingConnection();
            if (connection != nullptr) {
                DEBUG_LOG("out of accept");

                clientHandler = queue.registerEvent(connection->getSocket(), fp::EventType::READ, client);
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
    EXPECT_EQ(fp::OK, transport.sendMessage(sendMsg));
    DEBUG_LOG("..sent");
    server.join();
}