//
//  TFSocket.cpp
//  TFFIXEngine
//
//  Created by Tom Fewster on 02/04/2013.
//  Copyright (c) 2013 Tom Fewster. All rights reserved.
//

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

    Socket::Socket(const std::string &host, const std::string &service) throw(SocketException)
            : m_socket(-1), m_connected(false) {
        struct addrinfo hints;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET; //AF_UNSPEC; // use AF_INET6 to force IPv6
        hints.ai_socktype = SOCK_STREAM;

        if (host.size() == 0) {
            if (getaddrinfo(NULL, service.c_str(), &hints, &m_hostInfo) != 0) {
                ERROR_LOG("Failed to resolve service; service: " << service);
                throw SocketException("Failed to resolve host or service");
            }
        } else {
            if (getaddrinfo(host.c_str(), service.c_str(), &hints, &m_hostInfo) != 0) {
                ERROR_LOG("Failed to resolve host or service; Host: " << host << " service: " << service);
                throw SocketException("Failed to resolve host or service");
            }
        }

    }

    Socket::Socket(const std::string &host, const uint16_t &port) throw(SocketException)
            : m_port(port), m_socket(-1), m_connected(false) {

        std::ostringstream service;
        service << port;

        struct addrinfo hints;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
        hints.ai_socktype = SOCK_STREAM;

        if (host.size() == 0) {
            if (getaddrinfo(NULL, service.str().c_str(), &hints, &m_hostInfo) != 0) {
                ERROR_LOG("Failed to resolve service; service: " << port);
                throw SocketException("Failed to resolve service");
            }
        } else {
            if (getaddrinfo(host.c_str(), service.str().c_str(), &hints, &m_hostInfo) != 0) {
                ERROR_LOG("Failed to resolve host or service; Host: " << host << " service: " << port);
                throw SocketException("Failed to resolve host or service");
            }
        }
    }

    Socket::Socket(const struct hostent *host, const uint16_t &port) : m_port(port), m_socket(-1), m_connected(false) {
        memcpy(&m_host, host, sizeof(decltype(*host)));
    }

    Socket::Socket(const int socketFd, const bool connected) {
        m_socket = socketFd;
        m_connected = connected;
    }

    Socket::~Socket() {
        if (m_hostInfo != NULL) {
            freeaddrinfo(m_hostInfo);
        }

        if (m_connected) {
            close(m_socket);
        }
    }

    void Socket::setConnectionStateHandler(std::function<void(bool connected)> handler) {
        m_handler = handler;
    }

    void Socket::setSocket(const int socket, const bool isConnected) {
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

    void Socket::setOptions(int options) {
        m_options = options;

        int oldfl = fcntl(m_socket, F_GETFL);

        if ((options & SocketOptionsNonBlocking) == SocketOptionsNonBlocking) {
            fcntl(m_socket, F_SETFL, oldfl | O_NONBLOCK);
        } else {
            fcntl(m_socket, F_SETFL, oldfl & ~O_NONBLOCK);
        }

        if ((options & SocketOptionsDisableNagle) == SocketOptionsDisableNagle) {
            int flag = 1;
            if (setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, static_cast<void *>(&flag), sizeof(flag)) == -1) {
                // failed to turn off TCP delay
                ERROR_LOG("Failed to set TCP_NODELAY on FileDescriptor[" << m_socket << "]");
            }
        }

        if ((options & SocketOptionsHighPriority) == SocketOptionsHighPriority) {
#ifdef HAVE_IPTOS_LOWDELAY
            int tos = IPTOS_LOWDELAY;       /* see <netinet/ip.h> */
            if (setsockopt(m_socket, IPPROTO_IP, IP_TOS, &tos, sizeof(tos)) == -1) {
               WARNING_LOG("Failed to set socket priority to " << tos << " (do you have the CAP_NET_ADMIN capability?)");
            }
#else
            ERROR_LOG("Setting socket priority not supported");
#endif
        }

#ifdef HAVE_NOSIGPIPE
        if ((options & SocketOptionsDisableSigPipe) == SocketOptionsDisableSigPipe) {
            int flag = 1;
            if (setsockopt(m_socket, SOL_SOCKET, SO_NOSIGPIPE, static_cast<void *>(&flag), sizeof(flag)) == -1) {
                // failed to disagle SIG_PIPE interupts
                ERROR_LOG("Failed to set SO_NOSIGPIPE on FileDescriptor[" << m_socket << "]");
            }
        }
#endif
    }

    //void TFSocket::setBufferSize() {
    //	int optval = 0;
    //	int optlen = sizeof(optval);
    //	getsockopt(socket, SOL_SOCKET, SO_SNDBUF, (int *)&optval, &optlen);
    //}


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
        if (readSize > 0) {
            length = readSize;
        } else if (readSize == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                length = 0;
                result = NoData;
            } else {
                length = 0;
                disconnect();
                result = Closed;
            }
        } else if (readSize == 0) {
            // Remote end closed the socket
            length = 0;
            disconnect();
            result = Closed;
        }

        return result;
    }
}

