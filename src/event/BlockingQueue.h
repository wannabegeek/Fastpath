//
// Created by Tom Fewster on 01/03/2016.
//

#ifndef TFDCF_BLOCKINGQUEUE_H
#define TFDCF_BLOCKINGQUEUE_H

#include <utils/blocking_ringbuffer.h>
#include "Queue.h"
#include "TimerEvent.h"
#include "SharedQueue.h"

namespace DCF {
    /**
     * Queue for dispatching events to the registered callbacks.
     * This queue will wait for an event to be placed on the queue,
     * it will then dispatch the event to the relevant callback.
     */
    class BlockingQueue : public SharedQueue<tf::blocking_ringbuffer<Queue::queue_value_type, 4096>> {
    public:
        /**
         * Attempt to dispatch any pending events and return immediately.
         * @return OK, or and error if one occurred
         */
        inline const status try_dispatch() override {
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
        inline const status dispatch() override {
            status status = OK;
            queue_value_type dispatcher;
            m_queue.pop_wait(dispatcher);
            this->dispatch_event(dispatcher);
            return status;
        }

        /**
         * Dispatch an event, but only wait until timeout is reached.
         * If possible avoid using this call, since it can have a performance impact,
         * to implement the timeout it internally create a timer event to drop out of
         * the `dispatch()` call. For optimal performance, it is best to add a timer event
         * to the queue.
         * @param timeout The maximum time in which to wait for an event.
         * @return OK, or and error if one occurred
         */
        const status dispatch(const std::chrono::milliseconds &timeout) override {
            status status = OK;
            if ((status = this->try_dispatch()) == NO_EVENTS) {
                // Create a TimerEvent and add to the dispatch loop
                if (m_timeout != nullptr) {
                    m_timeout->setTimeout(timeout);
                } else {
                    m_timeout = this->registerEvent(timeout, [this](TimerEvent *event) {
                        // noop - this will cause us to drop out of the dispatch loop
                        this->unregisterEvent(m_timeout);
                        m_timeout = nullptr;
                    });
                }
                status = this->dispatch();
            }

            return status;
        }

    };
}
#endif //TFDCF_BLOCKINGQUEUE_H
