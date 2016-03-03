//
// Created by Tom Fewster on 29/02/2016.
//

#ifndef TFDCF_QUEUE_H
#define TFDCF_QUEUE_H

#include "messages/StorageTypes.h"
#include "Session.h"

namespace DCF {
    class Event;

    class Queue {
    public:
        using queue_value_type = std::function<void ()>;

        virtual ~Queue() {}

        virtual void dispatch() = 0;
        virtual void dispatch(const std::chrono::milliseconds &timeout) = 0;
        virtual const bool try_dispatch() = 0;
        virtual const size_t eventsInQueue() const noexcept = 0;
        virtual const bool __enqueue(queue_value_type &event) noexcept = 0;

        inline void __notifyEventManager() noexcept {
            Session::instance().m_eventManager.notify();
        }

        template <typename T> void __registerEvent(T &evt) {
            Session::instance().m_eventManager.registerHandler(evt);
            this->__notifyEventManager();
        }

        template <typename T> void __unregisterEvent(T &evt) {
            Session::instance().m_eventManager.unregisterHandler(evt);
            this->__notifyEventManager();
        }
    };
}

#endif //TFDCF_QUEUE_H
