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

        Queue *m_queue;

        std::atomic<bool> m_isRegistered = ATOMIC_VAR_INIT(false);
        bool m_pendingRemoval = false;
        std::atomic<uint16_t> m_awaitingDispatch = ATOMIC_VAR_INIT(0);

        virtual const bool isEqual(const Event &other) const noexcept = 0;
        inline void __pushDispatch() noexcept {
            m_awaitingDispatch++;
        }

        inline void __popDispatch() noexcept {
            m_awaitingDispatch--;
        }

    public:
        Event(Queue *queue) : m_queue(queue) {
        }

        Event(Event &&other) : m_queue(other.m_queue), m_isRegistered(static_cast<bool>(other.m_isRegistered)), m_pendingRemoval(other.m_pendingRemoval) {
        }

        Event(const Event &other) = delete;
        Event& operator=(Event const&) = delete;

        virtual ~Event() {}

        virtual const bool __notify(const EventType &eventType) noexcept = 0;
        virtual void __destroy() = 0;

        const bool isRegistered() const noexcept {
            return m_isRegistered;
        }

        void __setIsRegistered(const bool flag) {
            m_isRegistered = flag;
        }

        void __setPendingRemoval(const bool flag) {
            m_pendingRemoval = flag;
        }

        const bool __awaitingDispatch() const noexcept {
            return m_awaitingDispatch.load(std::memory_order_relaxed) != 0;
        }

        bool operator==(const Event &other) {
            return m_queue == other.m_queue
                    && m_isRegistered == other.m_isRegistered
                    && this->isEqual(other);
        }
    };
}

#endif //TFDCF_EVENT_H
