//
// Created by Tom Fewster on 01/03/2016.
//

#ifndef TFDCF_INLINEQUEUE_H
#define TFDCF_INLINEQUEUE_H

#include "Queue.h"
#include "EventManager.h"

namespace DCF {
    class InlineQueue : public Queue {
    private:
        EventManager &m_eventManager;
    public:
        InlineQueue(EventManager &eventManager) : m_eventManager(eventManager) {
        }

        virtual ~InlineQueue() { }

        const bool try_dispatch() override {
            return false;
        }

        void dispatch() override {
            m_eventManager.waitForEvent();
        }

        void dispatch(const std::chrono::microseconds &timeout) override {
            m_eventManager.waitForEvent(timeout);
        }

        const size_t eventsInQueue() const noexcept override {
            return 0;
        }

        const bool __enqueue(queue_value_type &dispatcher) noexcept override {
            dispatcher();
            return true;
        }
    };
}

#endif //TFDCF_INLINEQUEUE_H
