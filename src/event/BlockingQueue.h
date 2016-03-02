//
// Created by Tom Fewster on 01/03/2016.
//

#ifndef TFDCF_BLOCKINGQUEUE_H
#define TFDCF_BLOCKINGQUEUE_H

#include "Queue.h"
#include "utils/blockingconcurrentqueue.h"

namespace DCF {
    class BlockingQueue : public Queue {
    private:
        using QueueType = moodycamel::BlockingConcurrentQueue<queue_value_type>;

        QueueType m_queue;

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

        void dispatch(const std::chrono::microseconds &timeout) override {
        }

        const size_t eventInQueue() const override {
            return m_queue.size_approx();
        }

        const bool enqueue(queue_value_type &event) noexcept override {
            return m_queue.try_enqueue(event);
        }
    };
}
#endif //TFDCF_BLOCKINGQUEUE_H
