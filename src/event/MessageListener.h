//
// Created by Tom Fewster on 29/02/2016.
//

#ifndef TFDCF_LISTENER_H
#define TFDCF_LISTENER_H

#include <unordered_set>
#include <messages/StorageTypes.h>
#include <transport/Transport.h>
#include <messages/Message.h>
#include <tuple>
#include "Subscriber.h"
#include "MessageEvent.h"

namespace DCF {
    class Queue;
    class TCPTransport;
    class TransportIOEvent;

    struct TransportContainer {
        Transport *transport;
        std::unique_ptr<TransportIOEvent> event;

        TransportContainer(Transport *t, std::unique_ptr<TransportIOEvent> &&e);
    };

    class MessageListener {
    private:
        typedef std::vector<std::unique_ptr<MessageEvent>> ObserversType;
        std::unordered_map<const Transport *, ObserversType> m_observers;
        std::vector<std::unique_ptr<TransportContainer>> m_transportConnections;

        void subscribe(Transport *transport, const char *subject) noexcept;
        void unsubscribe(Transport *transport, const char *subject) noexcept;

        void handleMessage(const Transport *transport, Transport::MessageType &message);

        const bool registerTransport(Transport *transport, EventManager *eventManager);
    public:
        static MessageListener& instance(){
            static MessageListener instance;
            return instance;
        }

        ~MessageListener();

        status addObserver(Queue *queue, const Subscriber &subscriber, EventManager *eventManager);
        status removeObserver(Queue *queue, const Subscriber &subscriber);
    };
}

#endif //TFDCF_LISTENER_H
