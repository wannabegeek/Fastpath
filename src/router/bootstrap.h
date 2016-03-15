//
// Created by Tom Fewster on 11/03/2016.
//

#ifndef TFDCF_BOOTSTRAP_H
#define TFDCF_BOOTSTRAP_H

#include <iosfwd>
#include <memory>
#include <transport/SocketServer.h>
#include <event/InlineQueue.h>
#include "peer_connection.h"

namespace fp{
    class bootstrap {
    private:
        DCF::InlineQueue m_dispatchQueue;

        DCF::SocketServer m_server;
        bool m_shutdown = false;

        std::vector<std::unique_ptr<peer_connection>> m_connections;

        void message_handler(peer_connection *source, const subject<> &subject, DCF::ByteStorage &msgData);
        void disconnection_handler(peer_connection *connection);
    public:
        bootstrap(const std::string &interface, const std::string &service);
        ~bootstrap();

        void run();
    };
}

#endif //TFDCF_BOOTSTRAP_H
