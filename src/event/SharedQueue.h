//
// Created by Tom Fewster on 18/03/2016.
//

#ifndef TFDCF_SHAREDQUEUE_H
#define TFDCF_SHAREDQUEUE_H

#include <utils/tfringbuffer.h>
#include "Queue.h"

namespace DCF {
    template <typename T> class SharedQueue : public Queue {
    protected:
        using QueueType = T<queue_value_type, 4096>;

        QueueType m_queue;
        TimerEvent *m_timeout;

        SharedQueue() : m_timeout(nullptr) {
        }

        virtual ~SharedQueue() { }

        virtual inline void dispatch_event(queue_value_type &dispatcher) {
            Event *event = dispatcher.first;
            dispatcher.second();
            if (event->__pendingRemoval() && !event->__awaitingDispatch()) {
                auto it = m_registeredEvents.find(make_find_set_unique(event));
                assert(it != m_registeredEvents.end());
                INFO_LOG("Removing event from queue " << event);
                m_registeredEvents.erase(it);
            }
        }

    public:
        const size_t eventsInQueue() const noexcept override {
            return m_queue.size();
        }

        const bool __enqueue(queue_value_type &&event) noexcept override {
            return m_queue.push(event);
        }
    };
}

#endif //TFDCF_SHAREDQUEUE_H
