//
//  TFSocketServer.cpp
//  TFFIXEngine
//
//  Created by Tom Fewster on 09/04/2013.
//  Copyright (c) 2013 Tom Fewster. All rights reserved.
//

#include "SocketServer.h"

#include <utils/logger.h>

#include <unistd.h>
#include <sys/fcntl.h>
#include <netinet/tcp.h>

namespace DCF {
    SocketServer::~SocketServer() {
    }


    bool SocketServer::connect(SocketOptions options) noexcept {
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

    std::unique_ptr<Socket> SocketServer::acceptPendingConnection() noexcept {
        struct sockaddr_storage remoteaddr; // client address
        socklen_t addrlen;
        memset(&remoteaddr, 0, sizeof(struct sockaddr_storage));
        memset(&addrlen, 0, sizeof(socklen_t));

        int newSocketFd = ::accept(m_socket, reinterpret_cast<struct sockaddr *>(&remoteaddr), &addrlen);
        if (newSocketFd != -1) {
            return std::make_unique<Socket>(newSocketFd, true);
        } else {
            return nullptr;
        }
    }
}
