//
// Created by Tom Fewster on 23/05/2016.
//

#include "shm_server_transport.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "fastpath/utils/logger.h"

#include "fastpath/transport/sm/InterprocessNotifierServer.h"
#include "fastpath/SharedMemoryBuffer.h"
#include "fastpath/event/Queue.h"
#include "fastpath/utils/temp_directory.h"
#include "fastpath/messages/MessageCodec.h"
#include "fastpath/router/shm_peer_connection.h"
#include "fastpath/router/message_wrapper.h"


namespace fp {
    shm_server_transport::shm_server_transport(Queue *queue, const ConnectionCallback &connectionCallback, const std::string &interface, const std::string &service) noexcept
            : server_transport(queue), m_manager(std::string("fprouter_").append(service).c_str(), true), m_serverQueue("ServerQueue", m_manager) {

        std::string ipc_file = tf::get_temp_directory();
        ipc_file.append("fprouter_");
        ipc_file.append(service);

        m_notification_server = std::make_unique<InterprocessNotifierServer>(ipc_file.c_str(), std::bind(&shm_server_transport::connectionHandler, this, connectionCallback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        m_globalConnectionHandler = m_notification_server->createReceiverEvent(m_queue);
    }

    shm_server_transport::~shm_server_transport() {
        m_queue->unregisterEvent(m_globalConnectionHandler);
    }

    void shm_server_transport::connectionHandler(const ConnectionCallback &connectionCallback, InterprocessNotifierServer::notifier_type &&notifier, std::unique_ptr<UnixSocket> &&socket, int process_id) {
        INFO_LOG("Someone connected to shared memory transport");

        auto *peer = new shm_peer_connection(m_queue, std::move(notifier), std::move(socket), process_id, &m_manager, std::bind(&shm_server_transport::messageHandler, this, std::placeholders::_1));
        connectionCallback(this, std::unique_ptr<peer_connection>(peer));
    }


    void shm_server_transport::messageHandler(const std::function<void(const subject<> &, const message_wrapper &)> &peer_message_handler) noexcept {
        m_serverQueue.retrieve([&](auto &ptr, SharedMemoryManager::shared_ptr_type &shared_ptr) {

            const char *subject_ptr = nullptr;
            size_t subject_length = 0;
            uint8_t flags = 0;
            size_t msg_length = 0;

            ptr.mark();
            auto status = MessageCodec::addressing_details(ptr, &subject_ptr, subject_length, flags, msg_length);
            if (status == MessageCodec::CompleteMessage) {
                INFO_LOG("Subject is: " << std::string(subject_ptr, subject_length));
            } else {
                ERROR_LOG("Somehow SHM has given us an incomplete message");
            }
            ptr.resetRead();

            subject<> subject(subject_ptr);
            if (tf::unlikely(subject.is_admin())) {
                Message message;
                if (MessageCodec::decode(&message, ptr)) {
                    INFO_LOG("Received admin message: " << message);
//                    this->handle_admin_message(subject, message);
                } else {
                    ERROR_LOG("Failed to decode message: " << ptr);
                    status = MessageCodec::CorruptMessage;
                }
            } else {
                message_wrapper mw(ptr, shared_ptr);
                peer_message_handler(subject, mw);
            }
        });
    }
}