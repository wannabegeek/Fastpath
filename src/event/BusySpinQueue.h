//
// Created by Tom Fewster on 01/03/2016.
//

#ifndef TFDCF_BUSYSPINQUEUE_H_H
#define TFDCF_BUSYSPINQUEUE_H_H

#include "Queue.h"
#include "utils/concurrentqueue.h"

namespace DCF {
    class BusySpinQueue : public Queue {
    private:
        using QueueType = moodycamel::ConcurrentQueue<queue_value_type>;

        QueueType m_queue;
        moodycamel::ProducerToken m_producerToken;
        std::unique_ptr<TimerEvent> m_timeout;

    public:
        BusySpinQueue() : m_queue(10000), m_producerToken(m_queue) {
        }

        virtual ~BusySpinQueue() { }

        const bool try_dispatch() override {
            queue_value_type dispatcher[32];
            size_t count = 0;
            if ((count = m_queue.try_dequeue_bulk_from_producer(m_producerToken, &dispatcher[0], 32))) {
                for (size_t i = 0; i < count; ++i) {
                    dispatcher[i]();
                }
                return true;
            }
            return false;
        }

        void dispatch() override {
            queue_value_type dispatcher[32];
            size_t count = 0;
            while ((count = m_queue.try_dequeue_bulk_from_producer(m_producerToken, &dispatcher[0], 32)) == 0) {
                // no-op - we will spin trying to get from the queue
            }
            for (size_t i = 0; i < count; ++i) {
                dispatcher[i]();
            }
        }

        void dispatch(const std::chrono::milliseconds &timeout) override {
            queue_value_type dispatcher[32];
            size_t count = 0;
            if ((count = m_queue.try_dequeue_bulk_from_producer(m_producerToken, &dispatcher[0], 32)) == 0) {
                // Create a TimerEvent and add to the dispatch loop
                if (m_timeout) {
                    m_timeout->setTimeout(timeout);
                } else {
                    m_timeout = std::make_unique<TimerEvent>(static_cast<BusySpinQueue *>(this), timeout, [](const TimerEvent *) {
                        // noop - this will cause us to drop out of the dispatch loop
                    });
                }
                while ((count = m_queue.try_dequeue_bulk_from_producer(m_producerToken, &dispatcher[0], 32)) == 0) {
                    // no-op - we will spin trying to get from the queue
                }
            }
            // we may have exited due to the timer firing, but hey dispatch it anyway
            for (size_t i = 0; i < count; i++) {
                dispatcher[i]();
            }
        }

        const size_t eventsInQueue() const noexcept override {
            return m_queue.size_approx();
        }

        const bool __enqueue(queue_value_type &event) noexcept override {
            return m_queue.try_enqueue(m_producerToken, event);
        }
    };
}
#endif //TFDCF_BUSYSPINQUEUE_H_H
