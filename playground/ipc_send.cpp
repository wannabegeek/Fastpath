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

int main(int argc, char *argv[]) {

    LOG_LEVEL(tf::logger::debug);

    fp::Session::initialise();
    fp::BlockingQueue m_queue;

    fp::SHMTransport transport("http://tomfewster.com", "Sample");

    typedef tf::pool<fp::MutableMessage, tf::nulllock> PoolType;
    PoolType pool(3);

    try {

        bool shutdown = false;
        unsigned int counter = 0;
        m_queue.registerEvent(std::chrono::seconds(1), [&] (const fp::TimerEvent *event) {
            if (transport.valid()) {
                auto msg = pool.allocate();
                msg->setSubject("SOME.TEST.SUBJECT");
                msg->addScalarField("TEST", counter++);
                msg->addDataField("Name", "Tom");
                msg->addDataField("Name2", "Zac");

                fp::status s;
                if ((s = transport.sendMessage(*msg)) != fp::OK) {
                    ERROR_LOG("Failed to send messages: " << fp::str_status[s]);
                    shutdown = true;
                }
                DEBUG_LOG("Sent message");
                pool.release(msg);
            } else {
                INFO_LOG("Transport not valid - can't send");
            }
        });


        std::this_thread::sleep_for(std::chrono::seconds(2));
        while(!shutdown && m_queue.dispatch() == fp::OK);
        DEBUG_LOG("Event loop dropped out");
    } catch (const fp::socket_error &e) {
        std::cerr << "BOOM - it's broken" << std::endl;
    }
}
