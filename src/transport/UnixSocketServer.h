//
// Created by fewstert on 01/04/16.
//

#ifndef TFDCF_UNIXSOCKETSERVER_H
#define TFDCF_UNIXSOCKETSERVER_H

#include <memory>
#include "UnixSocket.h"

namespace DCF {
    class UnixSocketServer final : public UnixSocket {
    public:
        UnixSocketServer(const std::string &path);
        UnixSocketServer(UnixSocketServer &&other);

        virtual ~UnixSocketServer();

        bool connect(SocketOptions options) noexcept override;
        std::unique_ptr<UnixSocket> acceptPendingConnection() noexcept;
    };
}


#endif //TFDCF_UNIXSOCKETSERVER_H
