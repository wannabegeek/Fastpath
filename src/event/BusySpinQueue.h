//
// Created by Tom Fewster on 01/03/2016.
//

#ifndef TFDCF_BUSYSPINQUEUE_H_H
#define TFDCF_BUSYSPINQUEUE_H_H

#include "utils/tfringbuffer.h"
#include "Queue.h"
#include "TimerEvent.h"

namespace DCF {
    class BusySpinQueue : public Queue {
    private:
        using QueueType = tf::ringbuffer<queue_value_type, 4096>;

        QueueType m_queue;
        TimerEvent *m_timeout;

    public:
        BusySpinQueue() : m_timeout(nullptr) {
        }

        virtual ~BusySpinQueue() { }

        inline const status try_dispatch() override {
            if (tf::likely(this->isInitialised())) {
                status result = NO_EVENTS;
                queue_value_type dispatcher;
                while (m_queue.pop(dispatcher)) {
                    dispatcher.second();
                    result = OK;
                }
                return result;
            } else {
                return EVM_NOTRUNNING;
            }
        }

        inline const status dispatch() override {
            status status = OK;
            while ((status = this->try_dispatch()) == NO_EVENTS) {
                // no-op - we will spin trying to get from the queue
            }
            return status;
        }

        inline const status dispatch(const std::chrono::milliseconds &timeout) override {
            status status = OK;
            if ((status = this->try_dispatch()) == NO_EVENTS) {
                // Create a TimerEvent and add to the dispatch loop
                if (m_timeout) {
                    m_timeout->setTimeout(timeout);
                } else {
                    m_timeout = this->registerEvent(timeout, [this](TimerEvent *event) {
                        // noop - this will cause us to drop out of the dispatch loop
                        this->unregisterEvent(event);
                    });
                }
                status = this->dispatch();
            }

            return status;
        }

        const size_t eventsInQueue() const noexcept override {
            return m_queue.size();
        }

        const bool __enqueue(queue_value_type &&event) noexcept override {
            return m_queue.push(event);
        }
    };
}
#endif //TFDCF_BUSYSPINQUEUE_H_H
