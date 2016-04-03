//
// Created by Tom Fewster on 01/04/2016.
//

#include "TCPSocket.h"
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

namespace DCF {

    TCPSocket::TCPSocket(const std::string &host, const std::string &service) throw(socket_error) {
        struct addrinfo hints;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET; //AF_UNSPEC; // use AF_INET6 to force IPv6
        hints.ai_socktype = SOCK_STREAM;

        if (host.size() == 0) {
            if (getaddrinfo(NULL, service.c_str(), &hints, &m_hostInfo) != 0) {
                ERROR_LOG("Failed to resolve service; service: " << service);
                throw socket_error("Failed to resolve host or service");
            }
        } else {
            if (getaddrinfo(host.c_str(), service.c_str(), &hints, &m_hostInfo) != 0) {
                ERROR_LOG("Failed to resolve host or service; Host: " << host << " service: " << service);
                throw socket_error("Failed to resolve host or service");
            }
        }

    }

    TCPSocket::TCPSocket(const std::string &host, const uint16_t &port) throw(socket_error) : m_port(port) {

        std::ostringstream service;
        service << port;

        struct addrinfo hints;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
        hints.ai_socktype = SOCK_STREAM;

        if (host.size() == 0) {
            if (getaddrinfo(NULL, service.str().c_str(), &hints, &m_hostInfo) != 0) {
                ERROR_LOG("Failed to resolve service; service: " << port);
                throw socket_error("Failed to resolve service");
            }
        } else {
            if (getaddrinfo(host.c_str(), service.str().c_str(), &hints, &m_hostInfo) != 0) {
                ERROR_LOG("Failed to resolve host or service; Host: " << host << " service: " << port);
                throw socket_error("Failed to resolve host or service");
            }
        }
    }

    TCPSocket::TCPSocket(const struct hostent *host, const uint16_t &port) : m_port(port) {
        memcpy(&m_host, host, sizeof(decltype(*host)));
    }

    TCPSocket::TCPSocket(const int socketFd, const bool connected) : Socket(socketFd, connected) {
    }

    TCPSocket::~TCPSocket() {
        if (m_hostInfo != NULL) {
            freeaddrinfo(m_hostInfo);
        }
    }

    void TCPSocket::setOptions(int options) noexcept {
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
                WARNING_LOG(
                        "Failed to set socket priority to " << tos << " (do you have the CAP_NET_ADMIN capability?)");
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
}

