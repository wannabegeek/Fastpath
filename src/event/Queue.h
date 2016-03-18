//
// Created by Tom Fewster on 29/02/2016.
//

#ifndef TFDCF_QUEUE_H
#define TFDCF_QUEUE_H

#include <utils/optimize.h>
#include <unordered_set>
#include "messages/StorageTypes.h"
#include "Session.h"
#include "event/EventManager.h"

namespace DCF {
    class Event;

    class Queue {
    private:
        std::unordered_set<std::unique_ptr<Event>> m_registeredEvents;
    protected:
        // The default implementation returns the global event manager
        virtual inline EventManager *eventManager() {
            return Session::instance().m_eventManager.get();
        }

        virtual inline const bool isInitialised() const {
            return Session::instance().m_started;
        }

    public:
        using queue_value_type = std::pair<Event *, std::function<void ()>>;

        virtual ~Queue();

        virtual const status dispatch() = 0;
        virtual const status dispatch(const std::chrono::milliseconds &timeout) = 0;
        virtual const status try_dispatch() = 0;
        virtual const size_t eventsInQueue() const noexcept = 0;
        virtual const bool __enqueue(queue_value_type &&event) noexcept = 0;

        const size_t event_count() const {
            return m_registeredEvents.size();
        }

        virtual inline void __notifyEventManager() noexcept {
            this->eventManager()->notify();
        }

        IOEvent *registerEvent(const int fd, const EventType eventType, const std::function<void(IOEvent *, const EventType)> &callback);
        TimerEvent *registerEvent(const std::chrono::milliseconds &timeout, const std::function<void(TimerEvent *)> &callback);

        status unregisterEvent(IOEvent *event);
        status unregisterEvent(TimerEvent *event);
    };
}

#endif //TFDCF_QUEUE_H
