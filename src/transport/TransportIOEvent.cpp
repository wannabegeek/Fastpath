//
// Created by fewstert on 23/03/16.
//

#include <messages/Message.h>
#include "TransportIOEvent.h"

namespace DCF {
    TransportIOEvent::TransportIOEvent(const int fd, const EventType &eventType, const std::function<void(TransportIOEvent *event, const EventType type)> &callback) : IOEvent(nullptr, fd, eventType), m_callback(callback) {
    }

    const bool TransportIOEvent::isEqual(const Event &other) const noexcept {
        return IOEvent::isEqual(other);
    }

    const bool TransportIOEvent::__notify(const EventType &eventType) noexcept {
        m_callback(this, eventType);
        return true;
    }

    void TransportIOEvent::__destroy() {
    }

}