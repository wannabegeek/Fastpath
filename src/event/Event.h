//
// Created by Tom Fewster on 29/02/2016.
//

#ifndef TFDCF_EVENT_H
#define TFDCF_EVENT_H

#include <functional>
#include "Queue.h"
#include "EventType.h"

namespace DCF {
    class Queue;

    class Event {
    protected:

        Queue *m_queue;

        virtual const bool isEqual(const Event &other) const noexcept = 0;

    public:
        Event(Queue *queue) : m_queue(queue) {
        }

//        SessionEventRegistration(SessionEventRegistration &&other) = delete;
        Event(const Event &other) = delete;
//        SessionEventRegistration &operator=(const SessionEventRegistration &) = delete;

        virtual ~Event() {}

        virtual const bool notify(const EventType &eventType) noexcept = 0;

        bool operator==(const Event &other) {
            return this->isEqual(other);
        }
    };
}

#endif //TFDCF_EVENT_H
