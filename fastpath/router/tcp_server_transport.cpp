//
// Created by Tom Fewster on 23/05/2016.
//

#include "tcp_server_transport.h"
#include "fastpath/event/Queue.h"
#include "fastpath/event/DataEvent.h"
#include "fastpath/router/tcp_peer_connection.h"

namespace fp {
    tcp_server_transport::tcp_server_transport(Queue *queue, const ConnectionCallback &connectionCallback, const std::string &interface, const std::string &service) noexcept
            : server_transport(queue), m_server(interface, service) {

        if (m_server.connect(SocketOptionsDisableNagle | SocketOptionsDisableSigPipe | SocketOptionsNonBlocking)) {
            m_connectionAttempt = m_queue->registerEvent(m_server.getSocket(), EventType::READ, [&](DataEvent *event, const EventType eventType) {
                INFO_LOG("Someone has tried to connect");
                connectionCallback(this, std::unique_ptr<peer_connection>(new tcp_peer_connection(m_queue, m_server.acceptPendingConnection())));
            });
        }
        INFO_LOG("Waiting for TCP connections on " << interface << ":" << service);
    }

    tcp_server_transport::~tcp_server_transport() noexcept {
        m_server.disconnect();
        if (m_connectionAttempt != nullptr) {
            m_queue->unregisterEvent(m_connectionAttempt);
        }
    }
}