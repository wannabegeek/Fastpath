//
// Created by fewstert on 01/04/16.
//

#include <iostream>
#include <chrono>
#include <thread>
#include "fastpath/utils/logger.h"
#include "fastpath/event/InlineQueue.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <boost/interprocess/shared_memory_object.hpp>
#pragma GCC diagnostic pop
#include "IPC/InterprocessNotifierServer.h"
#include "fastpath/SharedMemoryBuffer.h"

int main(int argc, char *argv[]) {

    LOG_LEVEL(tf::logger::debug);
//    fp::Session::initialise();
//    fp::InlineQueue queue;

    boost::interprocess::shared_memory_object::remove("SharedMemoryTest");
    SharedMemoryBuffer buffer("SharedMemoryTest");

    fp::InlineEventManager evm;
    std::vector<tf::notifier> m_notifiers;

    bool shutdown = false;
    auto notificationHandler = [&](fp::TransportIOEvent *event, const fp::EventType type, tf::notifier *notifier) noexcept {
        DEBUG_LOG("Received notification..... " << event->fileDescriptor());
        if (!notifier->reset()) {
            INFO_LOG("Client disconnected " << event->fileDescriptor());
//            DEBUG_LOG("Setting shutdown flag " << strerror(errno));
//            shutdown = true;
            evm.unregisterHandler(event);
        } else {
            void *ptr = buffer.retrieve();
            INFO_LOG("Received data '" << reinterpret_cast<const char *>(ptr) << "'");
            buffer.deallocate(ptr);
        }
    };

    try {
        INFO_LOG("Started");
        std::unique_ptr<fp::TransportIOEvent> notification_handler;
        fp::InterprocessNotifierServer notifier([&](tf::notifier &&notifier) {
            int fd = notifier.read_handle();
            DEBUG_LOG("Need to add callback for " << fd);
            m_notifiers.push_back(std::move(notifier));

            notification_handler = std::make_unique<fp::TransportIOEvent>(fd, fp::EventType::READ, std::bind(notificationHandler, std::placeholders::_1, std::placeholders::_2, &m_notifiers.back()));

            evm.registerHandler(notification_handler.get());
        });

        auto event = notifier.createReceiverEvent();
        evm.registerHandler(event.get());

        while(!shutdown) {
            evm.waitForEvent();
        };
        DEBUG_LOG("Event loop dropped out");

    } catch (const fp::socket_error &e) {
        ERROR_LOG("BOOM - it's broken: " << e.what());
    }

//    fp::Session::m_shutdown();
}
