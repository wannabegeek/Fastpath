//
// Created by Tom Fewster on 03/04/2016.
//

#include <fastpath/transport/UnixSocketServer.h>
#include <fastpath/utils/logger.h>
#include <fastpath/Exception.h>
#include <cassert>
#include "InterprocessNotifierServer.h"

namespace DCF {
    InterprocessNotifierServer::InterprocessNotifierServer(std::function<void(tf::notifier &&notifier)> callback) : InterprocessNotifier(std::make_unique<UnixSocketServer>("test_unix")), m_callback(callback) {
        m_socket->setOptions(SocketOptionsNonBlocking);
        if (!m_socket->connect(DCF::SocketOptionsNone)) {
            ERROR_LOG("Failed to create connection");
            throw fp::exception("Failed to create server connection");
        }
    }

    std::unique_ptr<TransportIOEvent> InterprocessNotifierServer::createReceiverEvent() {
        return std::make_unique<TransportIOEvent>(m_socket->getSocket(), EventType::READ, [&](TransportIOEvent *event, const EventType type) {
            DEBUG_LOG("Something happened on socket");

            std::unique_ptr<UnixSocket> connection = reinterpret_cast<UnixSocketServer *>(m_socket.get())->acceptPendingConnection();

            if (connection) {
                INFO_LOG("Accepted connection");

                int fd[256];
                size_t max = 256;
                if (this->receive_fd(connection.get(), fd, max)) {
                    assert(max == 2);
                    m_callback(tf::notifier(fd));

                    for (size_t i = 0; i < max; i++) {
                        INFO_LOG("Extracted " << fd[i]);
                    }
                } else {
                    INFO_LOG("Received event not an fd");
                }
            } else {
                ERROR_LOG("Something went wrong");
            }
        });
    }
}

