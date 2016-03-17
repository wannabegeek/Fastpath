//
// Created by Tom Fewster on 01/03/2016.
//

#ifndef TFDCF_BLOCKINGQUEUE_H
#define TFDCF_BLOCKINGQUEUE_H

#include <utils/blocking_ringbuffer.h>
#include "Queue.h"
#include "TimerEvent.h"

namespace DCF {
    class BlockingQueue : public Queue {
    private:
        using QueueType = tf::blocking_ringbuffer<queue_value_type, 4096>;

        QueueType m_queue;
        std::unique_ptr<TimerEvent> m_timeout;

    public:
        BlockingQueue() {
        }

        virtual ~BlockingQueue() { }

        const bool try_dispatch() override {
            bool result = false;
            queue_value_type dispatcher;
            while (m_queue.pop(dispatcher)) {
                dispatcher();
                result = true;
            }
            return result;
        }

        void dispatch() override {
            queue_value_type dispatcher;
            m_queue.pop_wait(dispatcher);
            dispatcher();
        }

        void dispatch(const std::chrono::milliseconds &timeout) override {
            if (!this->try_dispatch()) {
                // Create a TimerEvent and add to the dispatch loop
                if (m_timeout) {
                    m_timeout->setTimeout(timeout);
                } else {
                    m_timeout = std::make_unique<TimerEvent>(timeout, [this](TimerEvent *event) {
                        // noop - this will cause us to drop out of the dispatch loop
                        this->__unregisterEvent(*event);
                    });
                    this->__registerEvent(*(m_timeout.get()));
                }
                this->dispatch();
            }
        }

        const size_t eventsInQueue() const noexcept override {
            return m_queue.size();
        }

        const bool __enqueue(queue_value_type &event) noexcept override {
            return m_queue.push(event);
        }

    };
}
#endif //TFDCF_BLOCKINGQUEUE_H
