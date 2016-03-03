#include <gtest/gtest.h>

#include <event/EventManager.h>
#include <event/IOEvent.h>
#include <event/InlineQueue.h>
#include <transport/SocketServer.h>
#include <transport/SocketClient.h>

#include <chrono>
#include <thread>
#include <unistd.h>

TEST(Socket, SimpleReadWrite) {

    std::thread server([&]() {
        DCF::SocketServer svr("localhost", "6969");
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
                std::cout << "Socket closed" << std::endl;
                break;
            }
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));
    DCF::SocketClient client("localhost", "6969");
    ASSERT_TRUE(client.connect());
    EXPECT_TRUE(client.send("tom", 4)); // send 4bytes (including \0)

    server.join();
}

TEST(TFSocket, NonBlockingReadWrite) {
    bool callbackFired = false;

    DCF::EventManager mgr;

    std::thread server([&]() {
        DCF::SocketServer svr("localhost", "6967");
        ASSERT_TRUE(svr.connect());
        std::shared_ptr<DCF::Socket> connection = svr.acceptPendingConnection();

        char buffer[16];
        int counter = 0;
        ssize_t size = 0;
        while(true) {
            DCF::Socket::ReadResult result = connection->read(buffer, 16, size);
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
                std::cout << "Socket closed" << std::endl;
                break;
            }
        }

        EXPECT_TRUE(connection->send("goodbye", 8));
    });

    std::this_thread::sleep_for(std::chrono::microseconds(100));
    DCF::SocketClient client("localhost", "6967");
    DCF::InlineQueue queue(mgr);


    ASSERT_TRUE(client.connect(DCF::SocketOptionsDisableSigPipe));
    EXPECT_NE(-1, client.getSocket());

    DCF::IOEvent handler(&queue, client.getSocket(), DCF::EventType::READ, [&](const DCF::IOEvent *event, DCF::EventType eventType) {
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
                std::cout << "Socket closed" << std::endl;
                break;
            }
        }
    });

    EXPECT_TRUE(client.send("hello", 6));

    queue.dispatch(std::chrono::seconds(5));
    EXPECT_TRUE(callbackFired);
    server.join();
}

TEST(TFSocket, NonBlockingServerReadWrite) {
    bool callbackFired = false;

    std::thread server([&]() {
        DCF::SocketServer svr("localhost", "6966");
        ASSERT_TRUE(svr.connect(DCF::SocketOptionsNonBlocking));
        ASSERT_NE(-1, svr.getSocket());

        DCF::EventManager serverMgr;
        std::shared_ptr<DCF::Socket> connection;
        bool finished = false;

        DCF::InlineQueue queue(serverMgr);

        auto client = [&](const DCF::IOEvent *event, int eventType) {
            EXPECT_EQ(DCF::EventType::READ, eventType);
            char buffer[16];
            int counter = 0;
            ssize_t size = 0;
            while (true) {
                DCF::Socket::ReadResult result = connection->read(buffer, 16, size);
                std::cout << "got stuff" << std::endl;
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
                    std::cout << "Server Socket closed" << std::endl;
                    break;
                }
            }

            EXPECT_TRUE(connection->send("goodbye", 8));
            finished = true;
        };


        DCF::IOEvent handler(&queue, svr.getSocket(), DCF::EventType::READ, [&](const DCF::IOEvent *event, int eventType) {
            EXPECT_EQ(DCF::EventType::READ, eventType);
            std::cout << "entering accept" << std::endl;
            connection = svr.acceptPendingConnection();
            if (connection != nullptr) {
                std::cout << "out of accept" << std::endl;

                DCF::IOEvent clientHandler(&queue, connection->getSocket(), DCF::EventType::READ, client);
                std::cout << "registered new client" << std::endl;
            }
        });

        for (int i = 0; !finished && i < 15; i++) {
            std::cout << "wait..." << std::endl;
            queue.dispatch(std::chrono::milliseconds(1000));
            std::cout << "...serviced event" << std::endl;
        }
    });

    DCF::EventManager mgr;
    DCF::InlineQueue queue(mgr);

    std::this_thread::sleep_for(std::chrono::microseconds(100));
    DCF::SocketClient client("localhost", "6966");

    ASSERT_TRUE(client.connect(DCF::SocketOptionsDisableSigPipe));
    EXPECT_NE(-1, client.getSocket());

    DCF::IOEvent handler(&queue, client.getSocket(), DCF::EventType::READ, [&](const DCF::IOEvent *event, int eventType) {
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
                std::cout << "Client Socket closed" << std::endl;
                break;
            }
        }
    });

    EXPECT_TRUE(client.send("hello", 6));

    queue.dispatch(std::chrono::seconds(5));
    EXPECT_TRUE(callbackFired);

    server.join();
}
