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
        std::unique_ptr<TimerEvent> m_timeout;

    public:
        BlockingQueue() : m_queue(10000) {
        }

        virtual ~BlockingQueue() { }

        const bool try_dispatch() override {
            queue_value_type dispatcher;
            if (m_queue.try_dequeue(dispatcher)) {
                dispatcher();
                return true;
            }
            return false;
        }

        void dispatch() override {
            queue_value_type dispatcher;
            m_queue.wait_dequeue(dispatcher);
            dispatcher();
        }

        void dispatch(const std::chrono::milliseconds &timeout) override {
            queue_value_type dispatcher;
            if (!m_queue.try_dequeue(dispatcher)) {
                // Create a TimerEvent and add to the dispatch loop
                if (m_timeout) {
                    m_timeout->setTimeout(timeout);
                } else {
                    m_timeout = std::make_unique<TimerEvent>(static_cast<BlockingQueue *>(this), timeout, [](const TimerEvent *) {
                        // noop - this will cause us to drop out of the dispatch loop
                    });
                }
                while (!m_queue.try_dequeue(dispatcher));
            }
            // we may have exited due to the timer firing, but hey dispatch it anyway
            dispatcher();
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
