//
// Created by Tom Fewster on 29/02/2016.
//

#ifndef TFDCF_EVENT_H
#define TFDCF_EVENT_H

#include <functional>
#include "Queue.h"
#include "EventType.h"
#include "Session.h"
#include <atomic>

namespace DCF {
    class Queue;

    class Event {
    protected:

        friend class Queue;

        Queue *m_queue;

        std::atomic<bool> m_isRegistered = ATOMIC_VAR_INIT(false);
        std::atomic<bool> m_awaitingDispatch = ATOMIC_VAR_INIT(false);
//        std::atomic<bool> m_inCallback = ATOMIC_VAR_INIT(false);

        bool m_active;

        virtual const bool isEqual(const Event &other) const noexcept = 0;

        void setQueue(Queue *queue) {
            m_queue = queue;
        }

    public:
        Event() : m_active(false) {
        }

        Event(Event &&other) : m_queue(other.m_queue), m_isRegistered(static_cast<bool>(other.m_isRegistered)) {
        }

        Event(const Event &other) = delete;
        Event& operator=(Event const&) = delete;

        virtual ~Event() {}

        virtual const bool __notify(const EventType &eventType) noexcept = 0;

        const bool isRegistered() const noexcept {
            return m_isRegistered;
        }

        void __setIsRegistered(const bool flag) {
            m_isRegistered = flag;
        }

        const bool __awaitingDispatch() const noexcept {
            return m_awaitingDispatch;
        }

        void __setAwaitingDispatch(const bool value) noexcept {
            m_awaitingDispatch = value;
        }

        bool operator==(const Event &other) {
            return m_queue == other.m_queue
                    && m_isRegistered == other.m_isRegistered
                    && this->isEqual(other);
        }
    };
}

#endif //TFDCF_EVENT_H
