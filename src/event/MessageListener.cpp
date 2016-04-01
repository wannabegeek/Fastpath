//
// Created by fewstert on 21/03/16.
//

#include "MessageListener.h"
#include "utils/logger.h"
#include "IOEvent.h"
#include "Queue.h"
#include "transport/TCPTransport.h"

namespace DCF {

    TransportContainer::TransportContainer(Transport *t, std::unique_ptr<TransportIOEvent> &&e) : transport(t), event(std::move(e)) {}


    void MessageListener::subscribe(Transport *transport, const char *subject) noexcept {
        Message msg;
        msg.setSubject("_FP.REGISTER.OBSERVER");
        msg.addDataField("subject", subject);
        msg.addScalarField("id", reinterpret_cast<uint64_t>(this));
        transport->sendMessage(msg);
    }

    void MessageListener::unsubscribe(Transport *transport, const char *subject) noexcept {
        Message msg;
        msg.setSubject("_FP.UNREGISTER.OBSERVER");
        msg.addDataField("subject", subject);
        msg.addScalarField("id", reinterpret_cast<uint64_t>(this));
        transport->sendMessage(msg);
    }

    MessageListener::~MessageListener() {}

    const bool MessageListener::registerTransport(Transport *transport, EventManager *eventManager) {
        if (transport->m_eventManager == nullptr) {
            // lock
            if (transport->m_eventManager == nullptr) {
                transport->m_eventManager = eventManager;
                std::unique_ptr<TransportContainer> container = std::make_unique<TransportContainer>(transport, transport->createReceiverEvent(std::bind(&MessageListener::handleMessage, this, std::placeholders::_1, std::placeholders::_2)));
                if (container->event) {
                    eventManager->registerHandler(container->event.get());
                }
                m_transportConnections.push_back(std::move(container));
            }
            // unlock
            return true;
        } else if (transport->m_eventManager != eventManager) {
            ERROR_LOG("Transport cannot be associated with a global queue and an inline dispatch queue");
        } else {
            return true;
        }

        return false;
    }


    void MessageListener::handleMessage(const Transport *transport, Transport::MessageType &message) {
        auto it = m_observers.find(transport);
        if (it != m_observers.end()) {
            fp::subject<> subject(message->subject());
            ObserversType &observers = it->second;
            std::for_each(observers.begin(), observers.end(), [&](auto &messageEvent) {
                if (messageEvent->subscriber()->is_interested(subject)) {
                    messageEvent->__notify(message);
                }
            });
        }
    }

    status MessageListener::addObserver(Queue *queue, const Subscriber &subscriber, EventManager *eventManager) {

        if (this->registerTransport(subscriber.transport(), eventManager)) {
            auto it = m_observers.find(subscriber.transport());
            if (it == m_observers.end()) {
                ObserversType subscribers;
                subscribers.emplace_back(std::make_unique<MessageEvent>(queue, &subscriber));
                m_observers.emplace(subscriber.transport(), std::move(subscribers));
            } else {
                ObserversType &subscribers = it->second;
                subscribers.emplace_back(std::make_unique<MessageEvent>(queue, &subscriber));
            }
            this->subscribe(subscriber.transport(), subscriber.subject());
        } else {
            return INVALID_TRANSPORT_STATE;
        }

        return OK;

    }

    status MessageListener::removeObserver(Queue *queue, const Subscriber &subscriber) {
        auto it = m_observers.find(subscriber.transport());
        if (it != m_observers.end()) {
//            ObserversType &subscribers = it->second;
//            auto it2 = std::find(subscribers.begin(), subscribers.end(), .......);
//            if (it2 != subscribers.end()) {
//                this->unsubscribe(subscriber.subject());
//                subscribers.erase(it2);
//                return OK;
//            }
//            this->unsubscribe(subscriber.transport(), subscriber.subject());

        }

        return CANNOT_DESTROY;
    }
}