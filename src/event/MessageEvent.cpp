//
// Created by Tom Fewster on 30/03/2016.
//

#include "MessageEvent.h"
#include "Queue.h"

namespace DCF {
    MessageEvent::MessageEvent(Queue *queue, const Subscriber *subscriber) : Event(queue), m_subscriber(subscriber) {
    }

    const bool MessageEvent::isEqual(const Event &other) const noexcept {
        if (typeid(other) == typeid(MessageEvent)) {
            const MessageEvent &f = static_cast<const MessageEvent &>(other);
            return m_subscriber == f.m_subscriber;
        }
        return false;
    }

    const bool MessageEvent::__notify(const EventType &eventType) noexcept {
        return false;
    }

    const bool MessageEvent::__notify(Transport::MessageType &message) noexcept {
        assert(m_queue != nullptr);
        this->__pushDispatch();
        return m_queue->__enqueue(QueueElement(this, std::bind(&Subscriber::__dispatch, m_subscriber, message)));
    }

    void MessageEvent::__destroy() noexcept {
        m_queue->removeSubscriber(*m_subscriber);
    }
}
