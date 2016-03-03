//
//  TFSocketClient.h
//  TFFIXEngine
//
//  Created by Tom Fewster on 09/04/2013.
//  Copyright (c) 2013 Tom Fewster. All rights reserved.
//

#ifndef __TFFIXEngine__TFSocketClient__
#define __TFFIXEngine__TFSocketClient__

#include "Socket.h"

namespace DCF {
    class SocketClient final : public Socket {
    public:
        SocketClient(const std::string &host, const std::string &service) throw(SocketException)
                : Socket(host, service) {
        }

        SocketClient(const std::string &host, const uint16_t &port) throw(SocketException) : Socket(host, port) {
        }


        virtual bool connect(SocketOptions options = SocketOptionsNone) noexcept override;
    };
}

#endif /* defined(__TFFIXEngine__TFSocketClient__) */
