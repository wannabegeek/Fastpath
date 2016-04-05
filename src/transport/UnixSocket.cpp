//
// Created by fewstert on 01/04/16.
//

#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <errno.h>
#include "UnixSocket.h"

namespace DCF {
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

    UnixSocket::UnixSocket(const int socketFd, const bool connected) : Socket(socketFd, connected) {
    }

    UnixSocket::UnixSocket(UnixSocket &&other) : Socket(std::move(other)), m_addr(other.m_addr) {
        DEBUG_LOG("moving");
    }

    UnixSocket::~UnixSocket() {
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