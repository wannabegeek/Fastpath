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

    boost::interprocess::shared_memory_object::remove("SharedMemoryTest2");
    SharedMemoryBuffer buffer("SharedMemoryTest2");

    fp::InlineEventManager evm;
    std::vector<std::unique_ptr<fp::notifier>> m_notifiers;

    bool shutdown = false;
    auto notificationHandler = [&](fp::TransportIOEvent *event, const fp::EventType type, fp::notifier *notifier) noexcept {
        DEBUG_LOG("Received notification..... " << event->fileDescriptor());
        DEBUG_LOG("Resetting fd..... " << notifier->read_handle());
        if (!notifier->reset()) {
            INFO_LOG("Client disconnected " << event->fileDescriptor());
            auto it = std::find_if(m_notifiers.begin(), m_notifiers.end(), [&notifier](const std::unique_ptr<fp::notifier> &n) {
                return n.get() == notifier;
            });
            if (it != m_notifiers.end()) {
                m_notifiers.erase(it);
            }
            evm.unregisterHandler(event);

            if (m_notifiers.size() == 0) {
                DEBUG_LOG("Setting shutdown flag no client left");
                shutdown = true;
            }

        } else {
            auto ptr = buffer.retrieve();
            INFO_LOG("Received data '" << ptr << "'");
//            buffer.deallocate(ptr);
        }
    };

    try {
        INFO_LOG("Started");
        fp::InterprocessNotifierServer notifier([&](std::unique_ptr<fp::notifier> &&notifier) {
            int fd = notifier->read_handle();
            DEBUG_LOG("Need to add callback for " << fd);
            DEBUG_LOG("FD contained in notifier " << notifier->read_handle());
            m_notifiers.push_back(std::move(notifier));
            DEBUG_LOG("FD contained in notifier after move " << m_notifiers.back()->read_handle());

            auto ptr = m_notifiers.back().get();
            auto notification_handler = std::make_unique<fp::TransportIOEvent>(fd, fp::EventType::READ, std::bind(notificationHandler, std::placeholders::_1, std::placeholders::_2, ptr));
            evm.registerHandler(notification_handler.release());
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
