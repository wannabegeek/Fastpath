//
// Created by Tom Fewster on 01/03/2016.
//

#ifndef TFDCF_BLOCKINGQUEUE_H
#define TFDCF_BLOCKINGQUEUE_H

#include "Queue.h"
#include "TimerEvent.h"
#include "utils/blockingconcurrentqueue.h"

namespace DCF {
    class BlockingQueue : public Queue {
    private:
        using QueueType = moodycamel::BlockingConcurrentQueue<queue_value_type>;

        QueueType m_queue;
        moodycamel::ProducerToken m_producerToken;
        std::unique_ptr<TimerEvent> m_timeout;

    public:
        BlockingQueue() : m_queue(10000), m_producerToken(m_queue) {
        }

        virtual ~BlockingQueue() { }

        const bool try_dispatch() override {
            queue_value_type dispatcher[32];
            size_t count = 0;
            if ((count = m_queue.try_dequeue_bulk(&dispatcher[0], 32))) {
                for (size_t i = 0; i < count; ++i) {
                    dispatcher[i]();
                }
                return true;
            }
            return false;
        }

        void dispatch() override {
            queue_value_type dispatcher[32];
            size_t count = m_queue.wait_dequeue_bulk(&dispatcher[0], 32);
            for (size_t i = 0; i < count; ++i) {
                dispatcher[i]();
            }
        }

        void dispatch(const std::chrono::milliseconds &timeout) override {
            queue_value_type dispatcher[32];
            size_t count = 0;
            if ((count = m_queue.try_dequeue_bulk(&dispatcher[0], 32)) == 0) {
                // Create a TimerEvent and add to the dispatch loop
                if (m_timeout) {
                    m_timeout->setTimeout(timeout);
                } else {
                    m_timeout = std::make_unique<TimerEvent>(static_cast<BlockingQueue *>(this), timeout, [](const TimerEvent *) {
                        // noop - this will cause us to drop out of the dispatch loop
                    });
                }
                count = m_queue.wait_dequeue_bulk(&dispatcher[0], 32);
            }
            // we may have exited due to the timer firing, but hey dispatch it anyway
            for (size_t i = 0; i < count; ++i) {
                dispatcher[i]();
            }
        }

        const size_t eventsInQueue() const noexcept override {
            return m_queue.size_approx();
        }

        const bool __enqueue(queue_value_type &event) noexcept override {
            return m_queue.try_enqueue(event);
        }
    };
}
#endif //TFDCF_BLOCKINGQUEUE_H
