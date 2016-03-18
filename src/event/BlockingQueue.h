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
    class BlockingQueue : public SharedQueue<tf::blocking_ringbuffer> {
    public:
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

        inline const status dispatch() override {
            status status = OK;
            queue_value_type dispatcher;
            m_queue.pop_wait(dispatcher);
            this->dispatch_event(dispatcher);
            return status;
        }

        const status dispatch(const std::chrono::milliseconds &timeout) override {
            status status = OK;
            if ((status = this->try_dispatch()) == NO_EVENTS) {
                // Create a TimerEvent and add to the dispatch loop
                m_timeout = this->registerEvent(timeout, [this](TimerEvent *event) {
                    // noop - this will cause us to drop out of the dispatch loop
                });
                status = this->dispatch();
                this->unregisterEvent(m_timeout);
            }

            return status;
        }

    };
}
#endif //TFDCF_BLOCKINGQUEUE_H
