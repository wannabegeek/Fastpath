//
// Created by Tom Fewster on 23/05/2016.
//

#ifndef FASTPATH_TCP_SERVER_TRANSPORT_H
#define FASTPATH_TCP_SERVER_TRANSPORT_H

#include "fastpath/router/server_transport.h"
#include "fastpath/transport/socket/TCPSocketServer.h"

namespace fp {
    class DataEvent;
    class tcp_peer_connection;

    class tcp_server_transport : public server_transport {
        TCPSocketServer m_server;
        DataEvent *m_connectionAttempt = nullptr;
    public:

        tcp_server_transport(Queue *queue, const ConnectionCallback &connectionCallback, const std::string &interface, const std::string &service) noexcept;
        ~tcp_server_transport() noexcept;
    };
}

#endif //FASTPATH_TCP_SERVER_TRANSPORT_H
