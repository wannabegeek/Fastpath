//
// Created by Tom Fewster on 18/03/2016.
//

#ifndef TFDCF_SHAREDQUEUE_H
#define TFDCF_SHAREDQUEUE_H

#include <utils/tfringbuffer.h>
#include "Queue.h"

namespace DCF {
    /**
     * Abstract base class for BusySpinQueue and BlockingQueue.
     */
    template <typename T> class SharedQueue : public Queue {
    protected:
        using QueueType = T;

        QueueType m_queue;
        TimerEvent *m_timeout;

        SharedQueue() : m_timeout(nullptr) {
        }

        virtual ~SharedQueue() { }

        virtual inline void dispatch_event(queue_value_type &dispatcher) {
            Event *event = dispatcher.event;
            dispatcher.function();
            if (event->__pendingRemoval() && !event->__awaitingDispatch()) {
                auto it = m_registeredEvents.find(make_find_set_unique(event));
                assert(it != m_registeredEvents.end());
                m_registeredEvents.erase(it);
            }
            dispatcher.clear();
        }

    public:
        /**
         * Get the number of events waiting to be consumed off the queue.
         * @return Number of pending events
         */
        const size_t eventsInQueue() const noexcept override {
            return m_queue.size();
        }

        /// @cond DEV
        /**
         * Enqueue callback to the queue, ready to be dispatched by client thread.
         * This should be called by the Session thread
         */
        const bool __enqueue(queue_value_type &&event) noexcept override {
            return m_queue.push(std::forward<queue_value_type>(event));
        }
        /// @endcond
    };
}

#endif //TFDCF_SHAREDQUEUE_H
