//
// Created by Tom Fewster on 01/03/2016.
//

#ifndef TFDCF_INLINEQUEUE_H
#define TFDCF_INLINEQUEUE_H

#include "Queue.h"
#include "InlineEventManager.h"

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

        virtual inline EventManager *eventManager() override {
            return &m_eventManager;
        }

        virtual inline const bool isInitialised() const override {
            return true;
        }

    public:
        /**
         * Attempt to dispatch any pending events and return immediately.
         * For an inline queue, there can't be any pending events, so this always returns NO_EVENTS.
         * @return false.
         */
        inline const status try_dispatch() override {
            return NO_EVENTS;
        }

        /**
         * Dispatch an event. Will block until one or more events have been dispatched
         * @return OK, or and error if one occurred
         */
        inline const status dispatch() override {
            m_eventManager.waitForEvent();
            return OK;
        }

        /**
         * Dispatch an event, but only wait until timeout is reached.
         * @param timeout The maximum time in which to wait for an event.
         * @return OK, or and error if one occurred
         */
        inline const status dispatch(const std::chrono::milliseconds &timeout) override {
            m_eventManager.waitForEvent(timeout);
            return OK;
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
            dispatcher.second();
            return true;
        }
        /// @endcond
    };
}

#endif //TFDCF_INLINEQUEUE_H
