//
// Created by Tom Fewster on 17/03/2016.
//

#include <Exception.h>
#include "IOEvent.h"
#include "TimerEvent.h"

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

}