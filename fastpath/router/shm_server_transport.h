//
// Created by Tom Fewster on 23/05/2016.
//

#ifndef FASTPATH_SHM_SERVER_TRANSPORT_H
#define FASTPATH_SHM_SERVER_TRANSPORT_H

#include "fastpath/router/server_transport.h"
#include "fastpath/SharedMemoryBuffer.h"
#include "fastpath/transport/sm/InterprocessNotifierServer.h"

namespace fp {
    class UnixSocket;
    class DataEvent;

    class shm_server_transport : public server_transport {

        SharedMemoryManager m_manager;
        SharedMemoryBuffer m_serverQueue;
        std::unique_ptr<InterprocessNotifierServer> m_notification_server;

        DataEvent *m_globalConnectionHandler = nullptr;

        void connectionHandler(const ConnectionCallback &connectionCallback, InterprocessNotifierServer::notifier_type &&notifier, std::unique_ptr<UnixSocket> &&socket, int process_id);
        void messageHandler(const std::function<void(const subject<> &, const message_wrapper &)> &peer_message_handler) noexcept;
    public:
        shm_server_transport(Queue *queue, const ConnectionCallback &connectionCallback, const std::string &interface, const std::string &service) noexcept;
        ~shm_server_transport();
    };
}

#endif //FASTPATH_SHM_SERVER_TRANSPORT_H
