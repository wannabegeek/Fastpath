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

#include "Socket.h"
#include "config.h"

#include <utils/logger.h>

#include <sstream>
#include <unistd.h>
#include <sys/fcntl.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <cstring>

//TFSocket::TFSocket(const std::string &host, const std::string &service, int socketFd) throw(TFSocketException) {
//	m_socket = socketFd;
//	m_connected = true;
//}

namespace DCF {

    Socket::Socket() noexcept : m_socket(-1), m_connected(false) {
    }

    Socket::Socket(Socket &&other) noexcept : m_socket(other.m_socket), m_connected(other.m_connected.load()), m_handler(std::move(other.m_handler)) {
        other.m_connected = false;
    }

    Socket::Socket(const int socketFd, const bool connected) noexcept {
        m_socket = socketFd;
        m_connected = connected;
    }

    Socket::~Socket() noexcept {
        if (m_connected) {
            close(m_socket);
        }
    }

    void Socket::setConnectionStateHandler(std::function<void(bool connected)> handler) noexcept {
        m_handler = handler;
    }

    void Socket::setSocket(const int socket, const bool isConnected) noexcept {
        m_socket = socket;
        m_connected = isConnected;
    }


    bool Socket::disconnect() noexcept {
        if (m_connected) {
            if (::close(m_socket) == 0) {
                m_connected = false;
                if (m_handler) {
                    m_handler(false);
                }
                return true;
            }
        }

        return false;
    }

    bool Socket::send(const char *data, ssize_t length) noexcept {
        if (m_connected) {
#ifdef MSG_NOSIGNAL
            ssize_t result = ::send(m_socket, data, length, (m_options & SocketOptionsDisableSigPipe) == SocketOptionsDisableSigPipe ? MSG_NOSIGNAL : 0);
#else
            ssize_t result = ::send(m_socket, data, length, 0);
#endif
            if (result == length) {
                return true;
            } else if (result >= 0) {
                // we failed to send everything, lets call recursivly to send the rest.
                return send(&data[length], length - result);
            }
        }

        disconnect();
        return false;
    }

    bool Socket::send(const std::string &data) noexcept {
        return send(data.c_str(), data.size());
    }

    bool Socket::send(const struct iovec *vec, const int &count) noexcept {
        if (m_connected) {
            struct iovec *first_vec = const_cast<struct iovec *>(vec);
            int remaining_count = count;

            ssize_t result = writev(m_socket, first_vec, remaining_count);
            if (result != 0) {
                size_t bytes_to_consume = result;
                while (bytes_to_consume > 0) {
                    if (bytes_to_consume >= first_vec->iov_len) {
                        /* consume entire vector element */
                        bytes_to_consume -= first_vec->iov_len;
                        remaining_count--;
                        first_vec++;
                    } else {
                        /* consume partial vector element */
                        first_vec->iov_len -= bytes_to_consume;
                        first_vec->iov_base = static_cast<char *>(first_vec->iov_base) + bytes_to_consume;
                        bytes_to_consume = 0;
                    }
                }
                return true;
            }
        }

        disconnect();
        return false;
    }

    const Socket::ReadResult Socket::read(std::string &data) noexcept {
        char buffer[1024] = {0};
        ssize_t length = 0;
        ReadResult result = read(buffer, 1024, length);
        if (result == MoreData) {
            data.append(buffer, length);
        }
        return result;
    }

    const Socket::ReadResult Socket::read(const char *data, size_t maxLength, ssize_t &length) noexcept {
        ReadResult result = MoreData;

        ssize_t readSize = ::recv(m_socket, const_cast<char *>(data), maxLength, MSG_DONTWAIT);
        switch(readSize) {
            case 0:
                length = 0;
                disconnect();
                result = Closed;
                break;
            case -1:
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    length = 0;
                    result = NoData;
                } else {
                    length = 0;
                    disconnect();
                    result = Closed;
                }
                break;
            default:
                length = readSize;
        }

        return result;
    }
}

