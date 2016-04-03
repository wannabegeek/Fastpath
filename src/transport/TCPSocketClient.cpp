//
//  TFSocketClient.cpp
//  TFFIXEngine
//
//  Created by Tom Fewster on 09/04/2013.
//  Copyright (c) 2013 Tom Fewster. All rights reserved.
//

#include "TCPSocketClient.h"
#include <unistd.h>
#include <sys/fcntl.h>
#include <netinet/tcp.h>

namespace DCF {
    bool TCPSocketClient::connect(SocketOptions options) noexcept {
        if (!m_connected) {
            m_options = options;
            struct addrinfo *p = nullptr;
            for (p = m_hostInfo; p != nullptr; p = p->ai_next) {
                if ((m_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                    continue;
                }

                if (::connect(m_socket, p->ai_addr, p->ai_addrlen) == -1) {
                    ::close(m_socket);
                    continue;
                }

                break;
            }

            if (p != nullptr) {
                m_connected = true;
                setOptions(options);

                if (m_handler) {
                    m_handler(true);
                }
            }

        }

        return m_connected;
    }
}