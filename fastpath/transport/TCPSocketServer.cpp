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

#include "fastpath/transport/TCPSocketServer.h"

#include "fastpath/utils/logger.h"

#include <unistd.h>
#include <sys/fcntl.h>
#include <netinet/tcp.h>

namespace fp {
    TCPSocketServer::~TCPSocketServer() {
    }


    bool TCPSocketServer::connect(SocketOptions options) noexcept {
        if (!m_connected) {
            struct addrinfo *p = nullptr;
            for (p = m_hostInfo; p != nullptr; p = p->ai_next) {
                if ((m_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                    continue;
                }

                int opt = 1;
                ::setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, static_cast<void *>(&opt), sizeof(int));
                if (::bind(m_socket, p->ai_addr, p->ai_addrlen) == -1) {
                    ::close(m_socket);
                    continue;
                }

                break;
            }

            if (p != nullptr) {
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
            } else {
                ERROR_LOG("Failed to initiate the connection; " << strerror(errno));
            }

        }

        return false;
    }

    std::unique_ptr<TCPSocket> TCPSocketServer::acceptPendingConnection() noexcept {
        struct sockaddr_storage remoteaddr; // client address
        socklen_t addrlen;
        memset(&remoteaddr, 0, sizeof(struct sockaddr_storage));
        memset(&addrlen, 0, sizeof(socklen_t));

        int newSocketFd = ::accept(m_socket, reinterpret_cast<struct sockaddr *>(&remoteaddr), &addrlen);
        if (newSocketFd != -1) {
            return std::make_unique<TCPSocket>(newSocketFd, true);
        } else {
            return nullptr;
        }
    }
}
