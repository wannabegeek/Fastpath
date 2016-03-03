//
//  TFSocketServer.h
//  TFFIXEngine
//
//  Created by Tom Fewster on 09/04/2013.
//  Copyright (c) 2013 Tom Fewster. All rights reserved.
//

#ifndef __TFFIXEngine__TFSocketServer__
#define __TFFIXEngine__TFSocketServer__

#include "Socket.h"
#include <vector>
#include <cstring>
#include <memory>

namespace DCF {
    class SocketServer final : public Socket {
    public:
        SocketServer(const std::string &host, const std::string &service) throw(SocketException)
                : Socket(host, service) {
        }

        SocketServer(const std::string &host, const uint16_t &port) throw(SocketException) : Socket(host, port) {
        }

        virtual ~SocketServer();

        virtual bool connect(SocketOptions options = SocketOptionsNone) noexcept override;

        std::unique_ptr<Socket> acceptPendingConnection() noexcept;
    };
}

#endif /* defined(__TFFIXEngine__TFSocketServer__) */
