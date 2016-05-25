//
// Created by Tom Fewster on 23/05/2016.
//

#ifndef FASTPATH_PEER_CONNECTION_H
#define FASTPATH_PEER_CONNECTION_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "fastpath/messages/subscription.h"
#include "fastpath/messages/Message.h"

namespace fp {
    class Queue;
    class message_wrapper;

    class peer_connection {
    public:
        typedef std::function<void(peer_connection *, const subject<> &, const message_wrapper &)> MessageHandlerType;
        typedef std::function<void(peer_connection *)> DisconnectionHandlerType;
    private:
        // TODO: these need to be calculated up-front
        static subject<> RegisterObserver() { return subject<>("_FP.REGISTER.OBSERVER"); }
        static subject<> UnregisterObserver() { return subject<>("_FP.UNREGISTER.OBSERVER"); }

        std::vector<subscription<>> m_subscriptions;

    protected:
        Queue *m_queue;

        MessageHandlerType m_messageHandler;
        DisconnectionHandlerType m_disconnectionHandler;

        void handle_admin_message(const subject<> subject, Message &message) noexcept;
    public:
        peer_connection(Queue *queue);
        peer_connection(peer_connection &&other) noexcept;
        virtual ~peer_connection() noexcept;

        void setMessageHandler(const MessageHandlerType &messageHandler);
        void setDisconnectionHandler(const DisconnectionHandlerType &disconnectionHandler);

        void add_subscription(const char *subject) noexcept;
        void remove_subscription(const char *subject) noexcept;
        bool is_interested(const subject<> &subject) const noexcept;

        virtual bool sendBuffer(const message_wrapper &buffer) noexcept = 0;
    };
}

#endif //FASTPATH_PEER_CONNECTION_H
