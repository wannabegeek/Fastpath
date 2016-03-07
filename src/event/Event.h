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

        std::atomic<bool> m_isActive;

        virtual const bool isEqual(const Event &other) const noexcept = 0;

        void setQueue(Queue *queue) {
            m_queue = queue;
        }

        void setActive(const bool value) {
            m_isActive = value;
        }
    public:
        Event() : m_isActive(false) {
        }

        Event(Queue *queue) : m_queue(queue), m_isActive(true) {
        }

        Event(Event &&other) : m_queue(other.m_queue), m_isActive(static_cast<bool>(other.m_isActive)) {
        }

        Event(const Event &other) = delete;
        Event& operator=(Event const&) = delete;

        virtual ~Event() {}

        const bool isActive() const noexcept {
            return m_isActive;
        }

        virtual const bool __notify(const EventType &eventType) noexcept = 0;

        bool operator==(const Event &other) {
            return m_queue == other.m_queue
                    && m_isActive == other.m_isActive
                    && this->isEqual(other);
        }
    };
}

#endif //TFDCF_EVENT_H
