//
// Created by Tom Fewster on 17/03/2016.
//

#include "Queue.h"
#include <Exception.h>
#include "DataEvent.h"
#include "TimerEvent.h"
#include "MessageListener.h"
#include "event/Subscriber.h"

namespace DCF {

    Queue::~Queue() {
        std::for_each(m_registeredEvents.begin(), m_registeredEvents.end(), [&](auto &event) {
            DEBUG_LOG("Destroying event " << event.get());
            event->__destroy();
        });
    }

    DataEvent *Queue::registerEvent(const int fd, const EventType eventType, const std::function<void(DataEvent *, const EventType)> &callback) {
        auto result = m_registeredEvents.emplace(make_set_unique<Event>(new DataEvent(this, fd, eventType, callback)));
        assert(result.second == true);
        DataEvent *event = reinterpret_cast<DataEvent *>(result.first->get());
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

    status Queue::updateEvent(TimerEvent *event) {
        EventManager *em = this->eventManager();
        if (em != nullptr) {
            em->updateHandler(event);
            return OK;
        }
        return EVM_NOTRUNNING;
    }

        status Queue::unregisterEvent(DataEvent *event) {
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

    status Queue::addSubscriber(const Subscriber &subscriber) {
        return MessageListener::instance().addObserver(this, subscriber, this->eventManager());
    }

    status Queue::removeSubscriber(const Subscriber &subscriber) {
        return MessageListener::instance().removeObserver(this, subscriber);
    }
}