/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include <fastpath/transport/socket/UnixSocketServer.h>
#include <fastpath/utils/logger.h>
#include <fastpath/Exception.h>
#include <cassert>
#include "InterprocessNotifierServer.h"

namespace fp {
    InterprocessNotifierServer::InterprocessNotifierServer(std::function<void(std::unique_ptr<fp::notifier> &&notifier, int)> callback) : InterprocessNotifier(std::make_unique<UnixSocketServer>("test_unix")), m_callback(callback) {
        m_socket->setOptions(SocketOptionsNonBlocking);
        if (!m_socket->connect(fp::SocketOptionsNone)) {
            ERROR_LOG("Failed to create connection");
            throw fp::exception("Failed to create server connection");
        }
    }

    void InterprocessNotifierServer::receivedClientConnection(std::unique_ptr<UnixSocket> &connection) {
        if (connection) {
            INFO_LOG("Accepted connection");

            int fd[256];
            size_t max = 256;
            int sending_pid = -1;
            if (this->receive_fd(connection.get(), fd, max, sending_pid)) {
                assert(max == 2);
                m_callback(std::make_unique<fp::notifier>(fd), sending_pid);

                for (size_t i = 0; i < max; i++) {
                    INFO_LOG("Received fd: " << fd[i] << " from process: " << sending_pid);
                }
            } else {
                INFO_LOG("Received event not an fd");
            }
        } else {
            ERROR_LOG("Something went wrong");
        }
    }

    std::unique_ptr<TransportIOEvent> InterprocessNotifierServer::createReceiverEvent() {
        return std::make_unique<TransportIOEvent>(m_socket->getSocket(), EventType::READ, [&](TransportIOEvent *event, const EventType type) {
            DEBUG_LOG("Something happened on socket");

            std::unique_ptr<UnixSocket> connection = reinterpret_cast<UnixSocketServer *>(m_socket.get())->acceptPendingConnection();
            receivedClientConnection(connection);
        });
    }
}

