//
// Created by fewstert on 01/04/16.
//

#ifndef TFDCF_UNIXSOCKETCLIENT_H
#define TFDCF_UNIXSOCKETCLIENT_H


#include "UnixSocket.h"

namespace DCF {
    class UnixSocketClient final : public UnixSocket {
    public:
        UnixSocketClient(const std::string &path);
        UnixSocketClient(UnixSocketClient &&other);

        bool connect(SocketOptions options = SocketOptionsNone) noexcept override;
    };
}


#endif //TFDCF_UNIXSOCKETCLIENT_H
