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
    class Message;
    class Subscriber;

    template <typename T> struct unique_ptr_deleter {
        bool m_owner;
        explicit unique_ptr_deleter(bool owner = true) : m_owner(owner) {}

        void operator()(T *p) const {
            if (m_owner) {
                delete p;
            }
        }
    };

    template<class T> using set_unique_ptr = std::unique_ptr<T, unique_ptr_deleter<T>>;

    template<class T> set_unique_ptr<T> make_find_set_unique(T *ptr){
        return set_unique_ptr<T>(ptr, unique_ptr_deleter<T>(false));
    }

    template<class T> set_unique_ptr<T> make_set_unique(T *ptr) {
        return set_unique_ptr<T>(ptr, unique_ptr_deleter<T>(true));
    }

//    template<class T, class... Args> set_unique_ptr<T> make_set_unique(Args&&... args) {
//        return set_unique_ptr<T>(new T(std::forward<Args>(args)...), unique_ptr_deleter<T>(true));
////        return std::make_unique<T, unique_ptr_deleter<T>()>(std::forward<Args>(args)...));
//    }

    class Queue {
    protected:
        std::unordered_set<set_unique_ptr<Event>> m_registeredEvents;

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

        status addSubscriber(const Subscriber &subscriber, const std::function<void(Subscriber *, Message *)> &callback);
        status removeSubscriber(const Subscriber &subscriber);
    };
}

#endif //TFDCF_QUEUE_H
