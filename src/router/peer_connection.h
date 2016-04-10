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
        // TODO: these need to be calculated up-front
        static subject<> RegisterObserver() { return subject<>("_FP.REGISTER.OBSERVER"); }
        static subject<> UnregisterObserver() { return subject<>("_FP.UNREGISTER.OBSERVER"); }

        std::vector<subscription<>> m_subscriptions;
        DCF::Queue *m_queue;
        std::unique_ptr<DCF::Socket> m_socket;
        DCF::DataEvent *m_socketEvent;

        DCF::MessageBuffer m_buffer;
        const std::function<void(peer_connection *, const subject<> &, const DCF::MessageBuffer::ByteStorageType &)> m_messageHandler;
        const std::function<void(peer_connection *)> m_disconnectionHandler;

        void data_handler(DCF::DataEvent *event, const DCF::EventType eventType) noexcept;
        DCF::MessageDecodeStatus process_buffer(const DCF::MessageBuffer::ByteStorageType &buffer) noexcept;

        void handle_admin_message(const subject<> subject, DCF::Message &message) noexcept;
    public:
        peer_connection(DCF::Queue *queue, std::unique_ptr<DCF::Socket> socket, const std::function<void(peer_connection *, const subject<> &, const DCF::MessageBuffer::ByteStorageType &)> messageHandler, const std::function<void(peer_connection *)> &disconnectionHandler);
        peer_connection(peer_connection &&other) noexcept;
        ~peer_connection() noexcept;

        void add_subscription(const char *subject) noexcept;
        void remove_subscription(const char *subject) noexcept;
        bool is_interested(const subject<> &subject) const noexcept;

        bool sendBuffer(const DCF::MessageBuffer::ByteStorageType &buffer) noexcept;
    };
}

#endif //TFDCF_PEER_CONNECTION_H
