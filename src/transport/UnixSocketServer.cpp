//
// Created by fewstert on 01/04/16.
//

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
