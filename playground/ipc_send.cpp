//
// Created by fewstert on 01/04/16.
//

#include <iostream>
#include "fastpath/transport/socket/UnixSocketClient.h"
#include <thread>
#include <fastpath/event/InlineEventManager.h>
#include <fastpath/event/Session.h>
#include <fastpath/event/BlockingQueue.h>
#include "fastpath/utils/logger.h"
#include "fastpath/utils/tfnulllock.h"
#include "fastpath/SharedMemoryBuffer.h"
#include "fastpath/messages/MutableMessage.h"
#include "fastpath/transport/SHMTransport.h"
#include "fastpath/transport/realm_transport.h"
#include "fastpath/event/Subscriber.h"

int main(int argc, char *argv[]) {

    LOG_LEVEL(tf::logger::debug);

    fp::Session::initialise();
    fp::BlockingQueue queue;

    auto transport = fp::make_realm_connection("shm://localhost:6969", "Sample");

    typedef tf::pool<fp::MutableMessage, tf::nulllock> PoolType;
    PoolType pool(3);

    try {

        bool shutdown = false;
        uint32_t counter = 0;
        queue.registerEvent(std::chrono::milliseconds(1000), [&] (const fp::TimerEvent *event) {
            if (transport->valid()) {
                auto msg = pool.allocate();
                msg->setSubject("SOME.TEST.SUBJECT");
                msg->addScalarField("TEST", counter++);
                msg->addDataField("Name", "Tom");
                msg->addDataField("Name2", "Zac");

                fp::status s;
                if ((s = transport->sendMessage(*msg)) != fp::OK) {
                    ERROR_LOG("Failed to send messages: " << fp::str_status[s]);
                    shutdown = true;
                }
                pool.release(msg);
            } else {
                INFO_LOG("Transport not valid - can't send");
            }
        });

        auto terminate = [&](const fp::SignalEvent *event, const int signal) noexcept {
            INFO_LOG("Shutdown signal caught...");
            shutdown = true;
        };
        queue.registerEvent(SIGINT, terminate);
        queue.registerEvent(SIGTERM, terminate);

        queue.addSubscriber(fp::Subscriber(transport, "SOME.TEST.REPLY", [&](const fp::Subscriber *event, const fp::Message *msg) noexcept {
            INFO_LOG(*msg);
        }));

        queue.addSubscriber(fp::Subscriber(transport, "SOME.TEST.SUBJECT", [&](const fp::Subscriber *event, const fp::Message *msg) noexcept {
            INFO_LOG(*msg);
        }));

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        while(!shutdown && queue.dispatch() == fp::OK)
            ;
        DEBUG_LOG("Event loop dropped out");
    } catch (const fp::socket_error &e) {
        std::cerr << "BOOM - it's broken" << std::endl;
    }
    fp::Session::destroy();

}
