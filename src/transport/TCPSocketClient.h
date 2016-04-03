//
//  TFSocketClient.h
//  TFFIXEngine
//
//  Created by Tom Fewster on 09/04/2013.
//  Copyright (c) 2013 Tom Fewster. All rights reserved.
//

#ifndef __TFFIXEngine__TFSocketClient__
#define __TFFIXEngine__TFSocketClient__

#include "TCPSocket.h"

namespace DCF {
    class TCPSocketClient final : public TCPSocket {
    public:
        TCPSocketClient(const std::string &host, const std::string &service) throw(socket_error)
                : TCPSocket(host, service) {
        }

        TCPSocketClient(const std::string &host, const uint16_t &port) throw(socket_error) : TCPSocket(host, port) {
        }


        virtual bool connect(SocketOptions options = SocketOptionsNone) noexcept override;
    };
}

#endif /* defined(__TFFIXEngine__TFSocketClient__) */
