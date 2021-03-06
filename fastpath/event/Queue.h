/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef FASTPATH_QUEUE_H
#define FASTPATH_QUEUE_H

#include <unordered_set>
#include <memory>

#include "fastpath/utils/optimize.h"
#include "fastpath/messages/StorageTypes.h"
#include "fastpath/event/Session.h"
#include "fastpath/event/EventManager.h"
#include "fastpath/event/EventType.h"

namespace fp {
    class Event;
    class DataEvent;
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

    // try to align this to a cache line to prevent false sharing
    struct alignas(64) QueueElement {
        using FnType = std::function<void ()>;
        Event *event;
        FnType function;
        QueueElement() noexcept {}
        QueueElement(Event *e, FnType &&fn) noexcept : event(e), function(std::move(fn)) {}
        QueueElement(QueueElement &&other) noexcept : event(other.event), function(std::move(other.function)) {}

        void clear() noexcept {
            event = nullptr;
            function = nullptr;
        }

        const QueueElement &operator=(const QueueElement &&other) noexcept {
            event = std::move(other.event);
            function = std::move(other.function);
            return *this;
        }

        const QueueElement &operator=(const QueueElement &other) noexcept {
            event = other.event;
            function = other.function;
            return *this;
        }
    };

    class Queue {
    protected:
        std::unordered_set<Event *> m_registeredEvents;

        // The default implementation returns the global event manager
        virtual inline EventManager *eventManager() noexcept {
            return Session::instance().m_eventManager.get();
        }

        virtual inline const bool isInitialised() const {
            return Session::instance().m_started;
        }

    public:
        using queue_value_type = QueueElement;

        Queue() noexcept {}

        virtual ~Queue() noexcept;

        virtual const status dispatch() noexcept = 0;
        virtual const status dispatch(const std::chrono::milliseconds &timeout) noexcept = 0;
        virtual const status try_dispatch() noexcept = 0;
        virtual const size_t eventsInQueue() const noexcept = 0;
        virtual const bool __enqueue(queue_value_type &&event) noexcept = 0;

        const size_t event_count() const noexcept {
            return m_registeredEvents.size();
        }

        virtual inline void __notifyEventManager() noexcept {
            this->eventManager()->notify();
        }

        DataEvent *registerEvent(const int fd, const EventType eventType, const std::function<void(DataEvent *, const EventType)> &callback) noexcept;
        TimerEvent *registerEvent(const std::chrono::microseconds &timeout, const std::function<void(TimerEvent *)> &callback) noexcept;
        SignalEvent *registerEvent(const int signal, const std::function<void(SignalEvent *, int)> &callback) noexcept;

        status updateEvent(TimerEvent *event) noexcept;

        status unregisterEvent(DataEvent *event) noexcept;
        status unregisterEvent(TimerEvent *event) noexcept;
        status unregisterEvent(SignalEvent *event) noexcept;

        status addSubscriber(const Subscriber &subscriber) noexcept;
        status removeSubscriber(const Subscriber &subscriber) noexcept;
    };
}

#endif //FASTPATH_QUEUE_H
