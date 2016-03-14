//
// Created by Tom Fewster on 11/03/2016.
//

#ifndef TFDCF_BOOTSTRAP_H
#define TFDCF_BOOTSTRAP_H

#include <iosfwd>
#include <transport/SocketServer.h>
#include <event/InlineQueue.h>
#include "peer_connection.h"

namespace fp{
    class bootstrap {
    private:
        DCF::InlineQueue m_dispatchQueue;

        DCF::SocketServer m_server;
        bool m_shutdown = false;

        std::vector<peer_connection> m_connections;
    public:
        bootstrap(const std::string &interface, const std::string &service);
        ~bootstrap();

        void run();
    };
}

#endif //TFDCF_BOOTSTRAP_H
