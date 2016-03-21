//
// Created by fewstert on 21/03/16.
//

#include "MessageListener.h"
#include "utils/logger.h"

namespace DCF {
    void MessageListener::subscribe(const char *subject) noexcept {
        Message msg;
        msg.setSubject("_FP.REGISTER.OBSERVER");
        msg.addDataField("subject", subject);
        msg.addScalarField("id", reinterpret_cast<uint64_t>(this));
        m_transport->sendMessage(msg);
    }

    void MessageListener::unsubscribe(const char *subject) noexcept {
        Message msg;
        msg.setSubject("_FP.UNREGISTER.OBSERVER");
        msg.addDataField("subject", subject);
        msg.addScalarField("id", reinterpret_cast<uint64_t>(this));
        m_transport->sendMessage(msg);
    }

    MessageListener::~MessageListener() {}

    status MessageListener::registerTransport(Transport *transport, const EventManager *eventManager) {

        if (transport->m_eventManager == nullptr) {
            transport->m_eventManager = eventManager;
            // lock

            // unlock
        } else if (transport->m_eventManager != eventManager) {
            ERROR_LOG("Transport cannot be associated with a global queue and an inline dispatch queue");
            return INVALID_TRANSPORT_STATE;
        }

        return OK;
    }

//    void MessageListener::dispatcher(Queue *queue, Subscriber *subscriber, Message *msg) {
//        //queue->__enqueue(std::make_pair(this, std::bind(&Subscriber::dispatch, subscriber, subscriber)));
//    }

    status MessageListener::addObserver(Queue *queue, const Subscriber &subscriber, const EventManager *eventManager) {

        auto status = this->registerTransport(subscriber.transport(), eventManager);

        if (status == OK) {
            auto it = m_observers.find(subscriber.transport());
            if (it == m_observers.end()) {
                m_observers.emplace(subscriber.transport(), ObserversType(1, &subscriber));
            } else {
                ObserversType &subscribers = it->second;
                subscribers.push_back(&subscriber);
            }
        }

//
//        auto callback = std::bind(&MessageListener::dispatcher, this, queue, subscriber, std::placeholder::_1);
//
//        auto result = m_observers.emplace(std::make_tuple(queue, &subscriber));
//        this->subscribe(subscriber.subject());
//
//        return result.second ? OK : CANNOT_CREATE;
        return status;
    }

    status MessageListener::removeObserver(Queue *queue, const Subscriber &subscriber) {
        auto it = m_observers.find(subscriber.transport());
        if (it != m_observers.end()) {
            ObserversType &subscribers = it->second;
            auto it2 = std::find(subscribers.begin(), subscribers.end(), .......);
            if (it2 != subscribers.end()) {
                this->unsubscribe(subscriber.subject());
                subscribers.erase(it2);
                return OK;
            }
        }

        return CANNOT_DESTROY;
    }
}