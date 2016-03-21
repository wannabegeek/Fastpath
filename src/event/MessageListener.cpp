//
// Created by fewstert on 21/03/16.
//

#include "MessageListener.h"
#include "utils/logger.h"
#include "IOEvent.h"
#include "transport/TCPTransport.h"

namespace DCF {

    TransportContainer::TransportContainer(Transport *t, std::unique_ptr<IOEvent> &&e) : transport(t), event(std::move(e)) {}


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

    std::unique_ptr<TransportContainer> MessageListener::registerTransport(Transport *transport, EventManager *eventManager) {
        if (transport->m_eventManager == nullptr) {
            // lock
            if (transport->m_eventManager == nullptr) {
                transport->m_eventManager = eventManager;
                std::unique_ptr<TransportContainer> container = std::make_unique<TransportContainer>(transport, transport->createReceiverEvent());
                // ^^^^^^^ we need to add MessageListener::handleMessage to be acke to receive message callbacks
                if (container->event) {
                    eventManager->registerHandler(container->event.get());
                }
                return container;
            }
            // unlock
        } else if (transport->m_eventManager != eventManager) {
            ERROR_LOG("Transport cannot be associated with a global queue and an inline dispatch queue");
        }

        return nullptr;
    }


    void MessageListener::handleMessage(const Transport *transport, Message *message) {
        auto it = m_observers.find(transport);
        if (it != m_observers.end()) {
            ObserversType &subscribers = it->second;
            std::for_each(subscribers.begin(), subscribers.end(), [&](auto &subscriber) {
                // is this subscriber interested?
                // dispatch to handler if it is...
            });
        }
    }

    status MessageListener::addObserver(Queue *queue, const Subscriber &subscriber, EventManager *eventManager) {

        auto container = this->registerTransport(subscriber.transport(), eventManager);
        if (container) {
            auto it = m_observers.find(subscriber.transport());
            if (it == m_observers.end()) {
                m_observers.emplace(subscriber.transport(), ObserversType(1, &subscriber));
            } else {
                ObserversType &subscribers = it->second;
                subscribers.push_back(&subscriber);
            }
            this->subscribe(subscriber.transport(), subscriber.subject());
        } else {
            return INVALID_TRANSPORT_STATE;
        }

        return OK;

//
//        auto callback = std::bind(&MessageListener::dispatcher, this, queue, subscriber, std::placeholder::_1);
//
//        auto result = m_observers.emplace(std::make_tuple(queue, &subscriber));
//        this->subscribe(subscriber.subject());
//
//        return result.second ? OK : CANNOT_CREATE;
//        return status;
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