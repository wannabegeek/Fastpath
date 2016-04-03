//
//  TFSocketServer.h
//  TFFIXEngine
//
//  Created by Tom Fewster on 09/04/2013.
//  Copyright (c) 2013 Tom Fewster. All rights reserved.
//

#ifndef __TFFIXEngine__TFSocketServer__
#define __TFFIXEngine__TFSocketServer__

#include "TCPSocket.h"
#include <vector>
#include <cstring>
#include <memory>

namespace DCF {
    class TCPSocketServer final : public TCPSocket {
    public:
        TCPSocketServer(const std::string &host, const std::string &service) throw(socket_error)
                : TCPSocket(host, service) {
        }

        TCPSocketServer(const std::string &host, const uint16_t &port) throw(socket_error) : TCPSocket(host, port) {
        }

        virtual ~TCPSocketServer();

        virtual bool connect(SocketOptions options = SocketOptionsNone) noexcept override;

        std::unique_ptr<TCPSocket> acceptPendingConnection() noexcept;
    };
}

#endif /* defined(__TFFIXEngine__TFSocketServer__) */
