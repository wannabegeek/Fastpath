//
// Created by Tom Fewster on 23/05/2016.
//

#ifndef FASTPATH_SERVER_TRANSPORT_H
#define FASTPATH_SERVER_TRANSPORT_H

#include "fastpath/router/peer_connection.h"

namespace fp {
    class Queue;

    class server_transport {
    protected:
        Queue *m_queue;
    public:
        typedef std::function<void(server_transport *, std::unique_ptr<peer_connection> &&)> ConnectionCallback;

        server_transport(Queue *queue) noexcept : m_queue(queue) {}
        virtual ~server_transport() noexcept = default;
    };
}

#endif //FASTPATH_SERVERTRANSPORT_H
