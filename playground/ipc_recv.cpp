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

    bool shutdown = false;
    auto notificationHandler = [&](DCF::TransportIOEvent *event, const DCF::EventType type, tf::notifier *notifier) noexcept {
        INFO_LOG("Received notification..... " << event->fileDescriptor());
        if (!notifier->reset()) {
            DEBUG_LOG("Setting shutdown flag " << strerror(errno));
            shutdown = true;
        }
    };

    try {
        INFO_LOG("Started");
        std::unique_ptr<DCF::TransportIOEvent> notification_handler;
        DCF::InterprocessNotifierServer notifier([&](tf::notifier &&notifier) {
            int fd = notifier.read_handle();
            DEBUG_LOG("Need to add callback for " << fd);
            m_notifiers.push_back(std::move(notifier));

            notification_handler = std::make_unique<DCF::TransportIOEvent>(fd, DCF::EventType::READ, std::bind(notificationHandler, std::placeholders::_1, std::placeholders::_2, &m_notifiers.back()));

            evm.registerHandler(notification_handler.get());
        });

        auto event = notifier.createReceiverEvent();
        evm.registerHandler(event.get());

        while(!shutdown) {
            evm.waitForEvent();
        };
        DEBUG_LOG("Event loop dropped out");

    } catch (const DCF::socket_error &e) {
        ERROR_LOG("BOOM - it's broken: " << e.what());
    }

//    DCF::Session::m_shutdown();
}
