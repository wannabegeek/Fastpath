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
#include <future>
#include "InterprocessNotifierServer.h"
#include <fastpath/event/Queue.h>

namespace fp {
    InterprocessNotifierServer::InterprocessNotifierServer(const char *identifier, std::function<void(notifier_type &&, std::unique_ptr<UnixSocket> &&, int)> callback) : InterprocessNotifier(std::make_unique<UnixSocketServer>(identifier)), m_callback(callback) {
        m_socket->setOptions(SocketOptionsNonBlocking);
        if (!m_socket->connect(fp::SocketOptionsNone)) {
            ERROR_LOG("Failed to create connection");
            throw fp::exception("Failed to create server connection");
        }
    }

    void InterprocessNotifierServer::receivedClientConnection(std::unique_ptr<UnixSocket> &&connection) noexcept {
        if (connection) {
            DEBUG_LOG("Accepted connection");

            int fd[256];
            size_t max = 256;
            int sending_pid = -1;
            if (this->receive_fd(connection.get(), fd, max, sending_pid)) {
                assert(max == 2);
                m_callback(std::make_tuple(std::make_unique<fp::notifier>(fd[0], direction::pipe_read), std::make_unique<fp::notifier>(fd[1], direction::pipe_write)), std::move(connection), sending_pid);
            } else {
                ERROR_LOG("Received event not an fd");
            }
        } else {
            ERROR_LOG("Something went wrong");
        }
        // if conneciton hasn't been moved to the callback, it will go out od scope here and get destructed
    }

    DataEvent *InterprocessNotifierServer::createReceiverEvent(Queue *queue) noexcept {
            return queue->registerEvent(m_socket->getSocket(), EventType::READ, [&](DataEvent *event, const EventType type) {
                    DEBUG_LOG("Something happened on socket");
                    receivedClientConnection(reinterpret_cast<UnixSocketServer *>(m_socket.get())->acceptPendingConnection());
                });
    }
}

