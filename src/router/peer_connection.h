//
// Created by Tom Fewster on 11/03/2016.
//

#ifndef TFDCF_PEER_CONNECTION_H
#define TFDCF_PEER_CONNECTION_H

#include <string>
#include <vector>

namespace fp {
    class peer_connection {
    private:
        std::vector<std::string> m_subscriptions;
    public:
        peer_connection();
        ~peer_connection();

        void add_subscription(const char *subject);
        void remove_subscription(const char *subject);
        bool is_interested(const char *subject) const;
    };
}

#endif //TFDCF_PEER_CONNECTION_H
