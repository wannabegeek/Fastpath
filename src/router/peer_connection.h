//
// Created by Tom Fewster on 11/03/2016.
//

#ifndef TFDCF_PEER_CONNECTION_H
#define TFDCF_PEER_CONNECTION_H

#include <string>
#include <vector>
#include <memory>
#include <event/IOEvent.h>
#include <MessageBuffer.h>
#include "subscription.h"

namespace DCF {
    class Socket;
    class Queue;
}

namespace fp {
    class peer_connection {
    private:
        std::vector<subscription<>> m_subscriptions;
        DCF::Queue *m_queue;
        std::unique_ptr<DCF::Socket> m_socket;
        DCF::IOEvent m_socketEvent;

        DCF::MessageBuffer m_buffer;
        const std::function<void(peer_connection *)> m_disconnectionHandler;

        void data_handler(DCF::IOEvent *event, const DCF::EventType eventType);
    public:
        peer_connection(DCF::Queue *queue, std::unique_ptr<DCF::Socket> socket, const std::function<void(peer_connection *)> &disconnectionHandler);
        peer_connection(peer_connection &&other);
        ~peer_connection();

        void add_subscription(const char *subject);
        void remove_subscription(const char *subject);
        bool is_interested(const subject<> &subject) const;
    };
}

#endif //TFDCF_PEER_CONNECTION_H
