//
// Created by Tom Fewster on 11/03/2016.
//

#ifndef TFDCF_PEER_CONNECTION_H
#define TFDCF_PEER_CONNECTION_H

#include <string>
#include <vector>
#include "subscription.h"

namespace DCF {
    class Socket;
}

namespace fp {
    class peer_connection {
    private:
        std::vector<subscription<>> m_subscriptions;
        std::unique_ptr<DCF::Socket> m_socket;
    public:
        peer_connection(std::unique_ptr<DCF::Socket> socket);
        peer_connection(peer_connection &&other);
        ~peer_connection();

        void add_subscription(const char *subject);
        void remove_subscription(const char *subject);
        bool is_interested(const char *subject) const;
    };
}

#endif //TFDCF_PEER_CONNECTION_H
