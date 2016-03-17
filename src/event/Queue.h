//
// Created by Tom Fewster on 29/02/2016.
//

#ifndef TFDCF_QUEUE_H
#define TFDCF_QUEUE_H

#include <utils/optimize.h>
#include "messages/StorageTypes.h"
#include "Session.h"
#include "event/EventManager.h"

namespace DCF {
    class Event;

    class Queue {
    protected:
        // The default implementation returns the global event manager
        virtual EventManager &eventManager() {
            return Session::instance().m_eventManager;
        }

        virtual inline const bool isInitialised() const {
            return Session::instance().m_started;
        }

    public:
        using queue_value_type = std::function<void ()>;

        virtual ~Queue() {}

        virtual const status dispatch() = 0;
        virtual const status dispatch(const std::chrono::milliseconds &timeout) = 0;
        virtual const status try_dispatch() = 0;
        virtual const size_t eventsInQueue() const noexcept = 0;
        virtual const bool __enqueue(queue_value_type &event) noexcept = 0;

        virtual inline void __notifyEventManager() noexcept {
            this->eventManager().notify();
        }

        template <typename T> status registerEvent(T &evt) {
            if (tf::unlikely(!evt.m_active)) {
                return ALREADY_ACTIVE;
            } else {
                evt.setQueue(this);
                this->eventManager().registerHandler(evt);
                return OK;
            }
        }

        template <typename T> status unregisterEvent(T &evt) {
            if (tf::unlikely(evt.m_active)) {
                return NOT_ACTIVE;
            } else {
                // TODO - remove any pending items from the queue
                evt.setQueue(nullptr);
                this->eventManager().unregisterHandler(evt);
                return OK;
            }
        }
    };
}

#endif //TFDCF_QUEUE_H
