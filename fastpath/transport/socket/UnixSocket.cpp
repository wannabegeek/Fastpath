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

#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <errno.h>

#include "UnixSocket.h"

namespace fp {
    UnixSocket::UnixSocket(const std::string &path) throw(socket_error) {
        memset(static_cast<void *>(&m_addr), 0, sizeof(struct sockaddr_un));
        m_addr.sun_family = AF_UNIX;
        if (path.size() < sizeof(m_addr.sun_path)) {
            strncpy(m_addr.sun_path, path.c_str(), sizeof(m_addr.sun_path) - 1);
        } else {
            throw socket_error("Path name too long");
        }

        m_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    }

    UnixSocket::UnixSocket(const int socketFd, const bool connected) noexcept : Socket(socketFd, connected) {
    }

    UnixSocket::UnixSocket(UnixSocket &&other) noexcept : Socket(std::move(other)), m_addr(other.m_addr) {
        DEBUG_LOG("moving");
    }

    UnixSocket::~UnixSocket() noexcept {
    }

    void UnixSocket::setOptions(int options) noexcept {
        m_options = options;

        int oldfl = fcntl(m_socket, F_GETFL);

        if ((options & SocketOptionsNonBlocking) == SocketOptionsNonBlocking) {
            fcntl(m_socket, F_SETFL, oldfl | O_NONBLOCK);
        } else {
            fcntl(m_socket, F_SETFL, oldfl & ~O_NONBLOCK);
        }

    }

    bool UnixSocket::send_ancillary(const struct msghdr *msg, int flags) noexcept {
        return ::sendmsg(m_socket, msg, flags) != -1;
    }

    const Socket::ReadResult UnixSocket::read_ancillary(struct msghdr *msg) noexcept {
        Socket::ReadResult result = MoreData;

        if (::recvmsg(m_socket, msg, 0) == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                result = NoData;
            } else {
                disconnect();
                result = Closed;
            }
        }

        return result;
    }
}