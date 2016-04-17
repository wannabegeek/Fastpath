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

#ifndef TFDCF_BUSYSPINQUEUE_H_H
#define TFDCF_BUSYSPINQUEUE_H_H

#include "fastpath/utils/tfringbuffer.h"
#include "fastpath/event/Queue.h"
#include "fastpath/event/TimerEvent.h"
#include "fastpath/event/SharedQueue.h"

namespace DCF {
    /**
     * Queue using busy spin wait for dispatching to registered callbacks.
     * This queue will spin in a loop using up CPU cycles waiting
     * for an event to be placed on the queue.
     * This should only be used for very latency sensitive applications
     * since it will consume 100% of a CPU while waiting for an event.
     * Once an event has been placed on the queue, it will dispatch the
     * event to the relevant callback.
     */
    class BusySpinQueue : public SharedQueue<tf::ringbuffer<Queue::queue_value_type, 4096>> {
    private:
        std::unique_ptr<TimerEvent> m_timeout;
    public:
        BusySpinQueue() {
            m_timeout = std::make_unique<TimerEvent>(this, std::chrono::microseconds(0), [&](TimerEvent *event) noexcept {
                // no-op
            });
        }

        virtual ~BusySpinQueue() noexcept { }

        /**
         * Attempt to dispatch any pending events and return immediately.
         * @return OK, or and error if one occurred
         */
        inline const status try_dispatch() noexcept override {
            if (tf::likely(this->isInitialised())) {
                status result = NO_EVENTS;
                queue_value_type dispatcher;
                while (m_queue.pop(dispatcher)) {
                    this->dispatch_event(dispatcher);
                    result = OK;
                }
                return result;
            } else {
                return EVM_NOTRUNNING;
            }
        }

        /**
         * Dispatch an event. Will block until one or more events have been dispatched
         * @return OK, or and error if one occurred
         */
        inline const status dispatch() noexcept override {
            status status = OK;
            while ((status = this->try_dispatch()) == NO_EVENTS) {
                // no-op - we will spin trying to get from the queue
            }
            return status;
        }

        /**
         * Dispatch an event, but only wait until timeout is reached.
         * If posible avoid using this call, since it can have a performance impact,
         * to implement the timeout it internally create a timer event to drop out of
         * the `dispatch()` call. For optimal performance, it is best to add a timer event
         * to the queue.
         * @param timeout The maximum time in which to wait for an event.
         * @return OK, or and error if one occurred
         */
        inline const status dispatch(const std::chrono::milliseconds &timeout) noexcept override {
            status status = OK;
            if ((status = this->try_dispatch()) == NO_EVENTS) {
                // Create a TimerEvent and add to the dispatch loop
                m_timeout->setTimeout(timeout);
                this->eventManager()->registerHandler(m_timeout.get());
                status = this->dispatch();
                this->eventManager()->unregisterHandler(m_timeout.get());
            }

            return status;
        }
    };
}
#endif //TFDCF_BUSYSPINQUEUE_H_H
