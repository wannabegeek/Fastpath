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
#include "fastpath/transport/sm/InterprocessNotifierServer.h"
#include "fastpath/SharedMemoryBuffer.h"
#include "fastpath/messages/MutableMessage.h"
#include "fastpath/messages/Message.h"
#include "fastpath/messages/MessageCodec.h"
#include "fastpath/utils/tfpool.h"
#include "fastpath/utils/tfnulllock.h"

struct PeerConnection {
    std::unique_ptr<fp::notifier> m_notifier;
    int m_process_id;

    std::unique_ptr<fp::SharedMemoryBuffer> m_clientQueue;

    PeerConnection(std::unique_ptr<fp::notifier> &&notifier, int process_id, fp::SharedMemoryManager *sm_manager)
            : m_notifier(std::forward<std::unique_ptr<fp::notifier>>(notifier)),
              m_process_id(process_id) {

        char queueName[32];
        ::sprintf(queueName, "ClientQueue_%i", process_id);
        m_clientQueue = std::make_unique<fp::SharedMemoryBuffer>(queueName, *sm_manager);
    }
};

int main(int argc, char *argv[]) {

    LOG_LEVEL(tf::logger::debug);

    boost::interprocess::shared_memory_object::remove("fprouter_6969");
    fp::SharedMemoryManager sm_manager("fprouter_6969");

    fp::SharedMemoryBuffer serverQueue("ServerQueue", sm_manager);

    fp::InlineEventManager evm;
    std::vector<PeerConnection> m_notifiers;

    typedef tf::pool<fp::Message, tf::nulllock> PoolType;
    PoolType pool(3);


    bool shutdown = false;
    auto notificationHandler = [&](fp::TransportIOEvent *event, const fp::EventType type, fp::notifier *notifier) noexcept {
        if (!notifier->reset()) {
            auto it = std::find_if(m_notifiers.begin(), m_notifiers.end(), [&notifier](const PeerConnection &p) {
                return p.m_notifier.get() == notifier;
            });
            if (it != m_notifiers.end()) {
                INFO_LOG("Client " << it->m_process_id << " disconnected " << event->fileDescriptor());
                m_notifiers.erase(it);
            }
            evm.unregisterHandler(event);

            if (m_notifiers.size() == 0) {
                DEBUG_LOG("Setting shutdown flag no clients left");
                shutdown = true;
            }

        } else {
            serverQueue.retrieve([&](auto &ptr) {

                const char *subject_ptr = nullptr;
                size_t subject_length = 0;
                uint8_t flags = 0;
                size_t msg_length = 0;

                ptr.mark();
                auto status = fp::MessageCodec::addressing_details(ptr, &subject_ptr, subject_length, flags, msg_length);
                if (status == fp::MessageCodec::CompleteMessage) {
                    INFO_LOG("Subject is: " << std::string(subject_ptr, subject_length));
                }
                ptr.resetRead();

                fp::Message *msg = pool.allocate();
                fp::MessageCodec::decode(msg, ptr);
                INFO_LOG("Received data '" << *msg);

                fp::MutableMessage msg2;
                msg2.setSubject("SOME.TEST.REPLY");
                uint32_t v = 0;
                msg->getScalarField("TEST", v);
                msg2.addScalarField("RESPONSE", v);
                msg2.addDataField("Name", "Tom");
                msg2.addDataField("Name2", "Zac");

                fp::SharedMemoryBuffer::mutable_storage_type storage(2048, sm_manager.allocator());
                fp::MessageCodec::encode(&msg2, storage);

                std::for_each(m_notifiers.begin(), m_notifiers.end(), [&storage](const PeerConnection &p) {
                    p.m_clientQueue->notify(&storage);
                    p.m_notifier->notify();
                });

                pool.release(msg);
            });
        }
    };

    try {
        INFO_LOG("Started");
        fp::InterprocessNotifierServer notifier([&](std::unique_ptr<fp::notifier> &&notifier, int process_id) {
            int fd = notifier->read_handle();
            DEBUG_LOG("Need to add callback for " << fd);
            m_notifiers.emplace_back(std::move(notifier), process_id, &sm_manager);

            auto ptr = m_notifiers.back().m_notifier.get();
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

    fp::Session::destroy();
}
