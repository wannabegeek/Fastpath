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

#ifndef TFDCF_INLINEQUEUE_H
#define TFDCF_INLINEQUEUE_H

#include "Queue.h"
#include "InlineEventManager.h"
#include "TimerEvent.h"

namespace DCF {
    /**
     * Queue for dispatching events to the registered callbacks.
     * This is a restricted dispatch mechanism only to be used for very
     * low latency requirements. This dispatches events directly on the IO
     * thread so it is essential that all the callback methods return as
     * quickly as possible since this could cause a build up of IO events.
     */
    class InlineQueue final : public Queue {
    private:
        InlineEventManager m_eventManager;
        std::unique_ptr<TimerEvent> m_timeout;

        virtual inline EventManager *eventManager() noexcept override {
            return &m_eventManager;
        }

        virtual inline const bool isInitialised() const noexcept override {
            return true;
        }

    public:
        InlineQueue() {
            m_timeout = std::make_unique<TimerEvent>(this, std::chrono::microseconds(0), [&](TimerEvent *event) noexcept {
                // no-op
            });
        }
        /**
         * Attempt to dispatch any pending events and return immediately.
         * For an inline queue, there can't be any pending events, so this always returns NO_EVENTS.
         * @return false.
         */
        inline const status try_dispatch() noexcept override {
            return NO_EVENTS;
        }

        /**
         * Dispatch an event. Will block until one or more events have been dispatched
         * @return OK, or and error if one occurred
         */
        inline const status dispatch() noexcept override {
            m_eventManager.waitForEvent();
            return OK;
        }

        /**
         * Dispatch an event, but only wait until timeout is reached.
         * @param timeout The maximum time in which to wait for an event.
         * @return OK, or and error if one occurred
         */
        inline const status dispatch(const std::chrono::milliseconds &timeout) noexcept override {
            status status = OK;
            if ((status = this->try_dispatch()) == NO_EVENTS) {
                // Create a TimerEvent and add to the dispatch loop
                m_timeout->setTimeout(timeout);
                m_eventManager.registerHandler(m_timeout.get());
                status = this->dispatch();
                m_eventManager.unregisterHandler(m_timeout.get());
            }

            return status;
        }

        /**
         * Get the number of events waiting to be consumed off the queue.
         * For an inline queue, this will always be 0.
         * @return Number of pending events
         */
        const size_t eventsInQueue() const noexcept override {
            return 0;
        }

        /// @cond DEV

        /**
         * No-op. We need to override the parents implementation so that we don't
         * do anything
         */
        virtual inline void __notifyEventManager() noexcept override {
        }

        const bool __enqueue(queue_value_type &&dispatcher) noexcept override {
            dispatcher.function();
            return true;
        }
        /// @endcond
    };
}

#endif //TFDCF_INLINEQUEUE_H
