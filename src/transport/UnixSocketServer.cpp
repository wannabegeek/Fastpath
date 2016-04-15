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

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <utils/logger.h>
#include <Exception.h>
#include "UnixSocketServer.h"

namespace DCF {
    UnixSocketServer::UnixSocketServer(const std::string &path) : UnixSocket(path) {
        if (::unlink(path.c_str()) == -1 && errno != ENOENT) {
            ThrowException(socket_error, "Failed to remove path " << strerror(errno));
        }
    }

    UnixSocketServer::UnixSocketServer(UnixSocketServer &&other) noexcept : UnixSocket(std::move(other)) {
    }

    UnixSocketServer::~UnixSocketServer() noexcept {
        ::unlink(m_addr.sun_path);
    }

    bool UnixSocketServer::connect(SocketOptions options) noexcept {
        if (!m_connected) {
            if (::bind(m_socket, reinterpret_cast<struct sockaddr *>(&m_addr), sizeof(struct sockaddr_un)) == -1) {
                ::close(m_socket);
                ERROR_LOG("Failed to initiate the connection; " << strerror(errno));
                return false;
            }

            m_connected = true;
            setOptions(options);

            if (::listen(m_socket, 10) == -1) {
                // Failed to listen
                ERROR_LOG("Failed to listen on server socket; " << strerror(errno));
                return false;
            }

            if (m_handler) {
                m_handler(true);
            }
            return true;
        }

        return false;
    }

    std::unique_ptr<UnixSocket> UnixSocketServer::acceptPendingConnection() noexcept {
        int newSocketFd = ::accept(m_socket, NULL, NULL);
        if (newSocketFd != -1) {
            return std::make_unique<UnixSocket>(newSocketFd, true);
        } else {
            ERROR_LOG("Failed to accept inbound connection: " << strerror(errno));
            return nullptr;
        }
    }
}
