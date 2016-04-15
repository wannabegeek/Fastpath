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

#ifndef TFDCF_SHAREDQUEUE_H
#define TFDCF_SHAREDQUEUE_H

#include "utils/tfringbuffer.h"
#include "TimerEvent.h"
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

        virtual ~SharedQueue() noexcept { }

        virtual inline void dispatch_event(queue_value_type &dispatcher) noexcept {
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
