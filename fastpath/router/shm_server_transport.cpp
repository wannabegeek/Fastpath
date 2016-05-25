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
            : server_transport(queue), m_manager(std::string("fprouter_").append(service).c_str(), true) {

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
        DEBUG_LOG("Someone connected to shared memory transport");

        auto *peer = new shm_peer_connection(m_queue, std::move(notifier), std::move(socket), process_id, &m_manager);
        connectionCallback(this, std::unique_ptr<peer_connection>(peer));
    }
}