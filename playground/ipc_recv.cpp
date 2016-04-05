//
// Created by fewstert on 01/04/16.
//

#include <iostream>
#include <chrono>
#include <thread>
#include <utils/logger.h>
#include <event/InlineQueue.h>
#include "IPC/InterprocessNotifierServer.h"

int main(int argc, char *argv[]) {

    LOG_LEVEL(tf::logger::debug);
//    DCF::Session::initialise();
//    DCF::InlineQueue queue;

    DCF::InlineEventManager evm;
    std::vector<tf::notifier> m_notifiers;

    try {
        INFO_LOG("Started");
        bool shutdown = false;
        std::unique_ptr<DCF::TransportIOEvent> notification_handler;
        DCF::InterprocessNotifierServer notifier([&](tf::notifier &&notifier) {
            DEBUG_LOG("Need to add callback for " << notifier.read_handle());

            notification_handler = std::make_unique<DCF::TransportIOEvent>(notifier.read_handle(), DCF::EventType::READ, [&](DCF::TransportIOEvent *event, const DCF::EventType type) {
                INFO_LOG("Received notification..... " << event->fileDescriptor());
                if (!notifier.reset()) {
                    shutdown = true;
                }
//                char buffer[1];
//                ssize_t readSize = ::read(event->fileDescriptor(), &buffer, 1);
//                switch (readSize) {
//                    case -1:
//                        ERROR_LOG("Error reading from pipe" << strerror(errno));
//                    case 0:
//                        INFO_LOG("Pipe closed by remote end");
//                        shutdown = true;
//                        break;
//                    default:
//                        INFO_LOG("Received: " << buffer[0]);
//                }
            });

            evm.registerHandler(notification_handler.get());
        });
        auto event = notifier.createReceiverEvent();
        evm.registerHandler(event.get());

        while(!shutdown) {
            evm.waitForEvent();
        };

    } catch (const DCF::socket_error &e) {
        ERROR_LOG("BOOM - it's broken: " << e.what());
    }

//    DCF::Session::m_shutdown();
}