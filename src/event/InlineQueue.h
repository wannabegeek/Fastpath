//
// Created by Tom Fewster on 01/03/2016.
//

#ifndef TFDCF_INLINEQUEUE_H
#define TFDCF_INLINEQUEUE_H

#include "Queue.h"
#include "InlineEventManager.h"

namespace DCF {
    class InlineQueue : public Queue {
    private:
        InlineEventManager m_eventManager;

        virtual inline EventManager *eventManager() override {
            return &m_eventManager;
        }

        virtual inline const bool isInitialised() const override {
            return true;
        }

    public:
        InlineQueue() {
        }

        virtual ~InlineQueue() { }

        inline const status try_dispatch() override {
            return NO_EVENTS;
        }

        inline const status dispatch() override {
            m_eventManager.waitForEvent();
            return OK;
        }

        inline const status dispatch(const std::chrono::milliseconds &timeout) override {
            m_eventManager.waitForEvent(timeout);
            return OK;
        }

        const size_t eventsInQueue() const noexcept override {
            return 0;
        }

        virtual inline void __notifyEventManager() noexcept override {
        }

        const bool __enqueue(queue_value_type &&dispatcher) noexcept override {
            dispatcher.second();
            return true;
        }
    };
}

#endif //TFDCF_INLINEQUEUE_H
