//
// Created by Tom Fewster on 01/03/2016.
//

#ifndef TFDCF_BUSYSPINQUEUE_H_H
#define TFDCF_BUSYSPINQUEUE_H_H

#include "utils/tfringbuffer.h"
#include "Queue.h"
#include "TimerEvent.h"
#include "SharedQueue.h"

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
