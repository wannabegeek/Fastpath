//
// Created by Tom Fewster on 17/03/2016.
//

#include <Exception.h>
#include "IOEvent.h"
#include "TimerEvent.h"
#include "transport/Transport.h"
#include "MessageListener.h"
#include "event/Subscriber.h"

namespace DCF {

    Queue::~Queue() {
        std::for_each(m_registeredEvents.begin(), m_registeredEvents.end(), [&](auto &event) {
            DEBUG_LOG("Destroying event " << event.get());
            event->__destroy();
        });
    }

    IOEvent *Queue::registerEvent(const int fd, const EventType eventType, const std::function<void(IOEvent *, const EventType)> &callback) {
        auto result = m_registeredEvents.insert(make_set_unique<Event>(new IOEvent(this, fd, eventType, callback)));
        assert(result.second == true);
        IOEvent *event = reinterpret_cast<IOEvent *>(result.first->get());
        EventManager *em = this->eventManager();
        if (em != nullptr) {
            em->registerHandler(event);
        } else {
            throw fp::exception("Event manager not running");
        }
        return event;
    }

    TimerEvent *Queue::registerEvent(const std::chrono::milliseconds &timeout, const std::function<void(TimerEvent *)> &callback) {
        auto result = m_registeredEvents.emplace(make_set_unique<Event>(new TimerEvent(this, timeout, callback)));
        TimerEvent *event = reinterpret_cast<TimerEvent *>(result.first->get());
        EventManager *em = this->eventManager();
        if (em != nullptr) {
            em->registerHandler(event);
        } else {
            throw fp::exception("Event manager not running");
        }
        return event;
    }

    status Queue::unregisterEvent(IOEvent *event) {
        // This will block any further callback to client code, which may still exist in the queue
        event->__setPendingRemoval(true);
        EventManager *em = this->eventManager();
        if (em != nullptr) {
            em->unregisterHandler(event);
            return OK;
        }
        return EVM_NOTRUNNING;
    }

    status Queue::unregisterEvent(TimerEvent *event) {
        // This will block any further callback to client code, which may still exist in the queue
        event->__setPendingRemoval(true);
        EventManager *em = this->eventManager();
        if (em != nullptr) {
            em->unregisterHandler(event);
            return OK;
        }
        return EVM_NOTRUNNING;
    }

    status Queue::addSubscriber(const Subscriber &subscriber, const std::function<void(Subscriber *, Message *)> &callback) {
        Transport *transport = subscriber.transport();
        if (transport->m_eventManager == nullptr || transport->m_eventManager == this->eventManager()) {
            if (transport->m_eventManager == nullptr) {
                transport->m_eventManager = this->eventManager();
                // registered with this->eventManager() & create message listener
                auto result = m_registeredTransports.emplace(transport, std::make_unique<MessageListener>());
                if (result.second == true) {
                    auto &messageListener = result.first;
                    messageListener->second->addObserver(subscriber);
                } else {
                    return CANNOT_CREATE;
                }
            } else {
                // add the subject to the MessageListener
                auto &messageListener = m_registeredTransports[transport];
                messageListener->addObserver(subscriber);
            }
        } else {
            // we must be associated with an inline queue - this is illegal
            ERROR_LOG("Transport cannot be associated with a global queue and an inline dispatch queue");
            return INVALID_TRANSPORT_STATE;
        }

        return OK;
    }

    status Queue::removeSubscriber(const Subscriber &subscriber) {
        // we should probably leave the transport still connected.
        auto it = m_registeredTransports.find(subscriber.transport());
        if (it != m_registeredTransports.end()) {
            (it->second)->removeObserver(subscriber);

            return OK;
        }

        return CANNOT_DESTROY;
    }

}