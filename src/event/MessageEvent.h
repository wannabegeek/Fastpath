//
// Created by Tom Fewster on 30/03/2016.
//

#ifndef TFDCF_MESSAGEEVENT_H
#define TFDCF_MESSAGEEVENT_H

#include <event/Event.h>
#include <transport/Transport.h>

namespace DCF {
    class Subscriber;

    class MessageEvent : public Event {
    private:
        const Subscriber *m_subscriber;

        virtual const bool isEqual(const Event &other) const noexcept override;
    public:
        MessageEvent(Queue *queue, const Subscriber *subscriber);

        virtual const bool __notify(const EventType &eventType) noexcept override;
        const bool __notify(Transport::MessageType &message) noexcept;
        virtual void __destroy() override;

        const Subscriber *subscriber() const noexcept { return m_subscriber; }
    };
}

#endif //TFDCF_MESSAGEEVENT_H
