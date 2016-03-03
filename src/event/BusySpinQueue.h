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

    public:
        BusySpinQueue() : m_queue(10000) {
        }

        virtual ~BusySpinQueue() { }

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
            while (!m_queue.try_dequeue(dispatcher));
            dispatcher();
        }

        void dispatch(const std::chrono::microseconds &timeout) override {
            queue_value_type dispatcher;
            if (!m_queue.try_dequeue(dispatcher)) {
                // TODO: create a TimerEvent and add to the dispatch loop
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
#endif //TFDCF_BUSYSPINQUEUE_H_H
