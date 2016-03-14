//
// Created by Tom Fewster on 11/03/2016.
//

#include "peer_connection.h"
#include "subject.h"
#include "transport/Socket.h"

namespace fp {
    peer_connection::peer_connection(std::unique_ptr<DCF::Socket> socket) : m_socket(std::move(socket)) {

    }

    peer_connection::peer_connection(peer_connection &&other) : m_subscriptions(std::move(other.m_subscriptions)), m_socket(std::move(other.m_socket)) {

    }

    peer_connection::~peer_connection() {

    }

    void peer_connection::add_subscription(const char *subject) {
        m_subscriptions.emplace_back(subject);
    }

    void peer_connection::remove_subscription(const char *subject) {
        auto it = std::find_if(m_subscriptions.begin(), m_subscriptions.end(), [&](const subscription<> &s) {
            return s == subject;
        });
        if (it != m_subscriptions.end()) {
//        m_subscriptions.erase(it);
        }
    }

    bool peer_connection::is_interested(const char *subject) const {
        auto it = std::find_if(m_subscriptions.begin(), m_subscriptions.end(), [&](const subscription<> &s) {
            return s.matches(subject);
        });

        return (it != m_subscriptions.end());
    }
}