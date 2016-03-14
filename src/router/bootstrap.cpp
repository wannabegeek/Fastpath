//
// Created by Tom Fewster on 11/03/2016.
//

#include <event/IOEvent.h>
#include "bootstrap.h"

namespace fp {
    bootstrap::bootstrap(const std::string &interface, const std::string &service) : m_server(interface, service) {
        INFO_LOG("Waiting for TCP connections on " << interface << ":" << service);
    }

    bootstrap::~bootstrap() {
    }

    void bootstrap::run() {
        // Start server socket listening
        m_server.connect(DCF::SocketOptionsDisableNagle | DCF::SocketOptionsDisableSigPipe | DCF::SocketOptionsNonBlocking);

        DCF::IOEvent connectionAttempt(&m_dispatchQueue, m_server.getSocket(), DCF::EventType::READ, [&](DCF::IOEvent *event, const DCF::EventType eventType) {
            INFO_LOG("Someone has tried to connect");
            m_connections.emplace_back(m_server.acceptPendingConnection());
        });


        // todo:
        // Start heartbeat thread
        // Start broadcast transport

        while (!m_shutdown) {
            m_dispatchQueue.dispatch();
            DEBUG_LOG("Dispatched an event");
        }

        DEBUG_LOG("Shutting down");
    }
}
