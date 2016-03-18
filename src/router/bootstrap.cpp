//
// Created by Tom Fewster on 11/03/2016.
//

#include <event/IOEvent.h>
#include <messages/Message.h>
#include "bootstrap.h"

namespace fp {
    bootstrap::bootstrap(const std::string &interface, const std::string &service) : m_server(interface, service) {
        INFO_LOG("Waiting for TCP connections on " << interface << ":" << service);
    }

    bootstrap::~bootstrap() {
    }

    void bootstrap::run() {
        // Start server socket listening
        if (m_server.connect(DCF::SocketOptionsDisableNagle | DCF::SocketOptionsDisableSigPipe | DCF::SocketOptionsNonBlocking)) {
            DCF::IOEvent *connectionAttempt = m_dispatchQueue.registerEvent(m_server.getSocket(), DCF::EventType::READ, [&](DCF::IOEvent *event, const DCF::EventType eventType) {
                INFO_LOG("Someone has tried to connect");
                m_connections.emplace_back(std::make_unique<peer_connection>(&m_dispatchQueue,
                                                                             m_server.acceptPendingConnection(),
                                                                             std::bind(&bootstrap::message_handler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
                                                                             std::bind(&bootstrap::disconnection_handler, this, std::placeholders::_1)));
            });

            // todo:
            // Start heartbeat thread
            // Start broadcast transport

            while (!m_shutdown) {
                m_dispatchQueue.dispatch();
            }

            m_dispatchQueue.unregisterEvent(connectionAttempt);
        }
        DEBUG_LOG("Shutting down");
    }

    void bootstrap::message_handler(peer_connection *source, const subject<> &subject, DCF::ByteStorage &msgData) {
        DEBUG_LOG("Processing message");
        // send the message out to all local client who are interested
        std::for_each(m_connections.begin(), m_connections.end(), [&](auto &connection) {
            if (connection->is_interested(subject)) {
                DEBUG_LOG("Connection is interested");
                // dispatch the message
                connection->sendBuffer(msgData);
            } else {
                DEBUG_LOG("Connection not interested");
            }
        });
    }

    void bootstrap::disconnection_handler(peer_connection *connection) {
        auto it = std::find_if(m_connections.begin(), m_connections.end(), [&](auto &c) {
            return c.get() == connection;
        });
        if (it != m_connections.end()) {
            m_connections.erase(it);
        } else {
            ERROR_LOG("Received disconnect notification for unknown connection");
        }
    }
}
