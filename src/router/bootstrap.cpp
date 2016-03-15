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
            DCF::IOEvent connectionAttempt(&m_dispatchQueue, m_server.getSocket(), DCF::EventType::READ, [&](DCF::IOEvent *event, const DCF::EventType eventType) {
                INFO_LOG("Someone has tried to connect");
                m_connections.emplace_back(std::make_unique<peer_connection>(&m_dispatchQueue, m_server.acceptPendingConnection(), std::bind(&bootstrap::disconnection_handler, this, std::placeholders::_1)));
            });


            // todo:
            // Start heartbeat thread
            // Start broadcast transport

            while (!m_shutdown) {
                m_dispatchQueue.dispatch();
            }
        }
        DEBUG_LOG("Shutting down");
    }

    void bootstrap::message_handler(DCF::MessageType &msg) {
        // send the message out to all local client who are interested
        const subject<> s(msg->subject());
        std::for_each(m_connections.begin(), m_connections.end(), [&](auto &connection) {
            if (connection->is_interested(s)) {
                // dispatch the message
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
