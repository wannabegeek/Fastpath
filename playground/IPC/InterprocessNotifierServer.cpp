//
// Created by Tom Fewster on 03/04/2016.
//

#include <transport/UnixSocketServer.h>
#include <utils/logger.h>
#include <Exception.h>
#include "InterprocessNotifierServer.h"

namespace DCF {
    InterprocessNotifierServer::InterprocessNotifierServer() : InterprocessNotifier(std::make_unique<UnixSocketServer>("test_unix")) {
        if (!m_socket->connect(DCF::SocketOptionsNone)) {
            ERROR_LOG("Failed to create connection");
            throw fp::exception("Failed to create server connection");
        }
    }

    std::unique_ptr<TransportIOEvent> InterprocessNotifierServer::createReceiverEvent() {
        return std::make_unique<TransportIOEvent>(m_socket->getSocket(), EventType::READ, [&](TransportIOEvent *event, const EventType type) {
            DEBUG_LOG("Something happened on socket");

            std::unique_ptr<DCF::UnixSocket> connection = reinterpret_cast<UnixSocketServer &>(m_socket).acceptPendingConnection();

            if (connection) {
                INFO_LOG("Accepted connection");

                int fd[256];
                size_t max = 256;
                this->receive_fd(fd, max);
                for (size_t i = 0; i < max; i++) {
                    INFO_LOG("Extracted " << fd[i]);
                }
            } else {
                ERROR_LOG("Something went wrong");
            }
        });
    }
}

