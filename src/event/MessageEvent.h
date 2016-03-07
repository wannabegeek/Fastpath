//
// Created by Tom Fewster on 29/02/2016.
//

#ifndef TFDCF_LISTENER_H
#define TFDCF_LISTENER_H

#include <messages/StorageTypes.h>
#include <transport/Transport.h>
#include "Event.h"

namespace DCF {
    class MessageEvent : public Event {

        const char *m_subject[std::numeric_limits<uint16_t>::max()];
        std::function<void(const MessageEvent *, const MessageType msg)> m_callback;

    public:
        MessageEvent(const Queue &queue, Transport *transport, const char *subject, const std::function<void(const MessageEvent *, const MessageType msg)> &callback) : Event(queue), m_callback(callback) {
            std::copy(subject, &subject[std::numeric_limits<uint16_t>::max()], m_subject);
        };

        const bool isEqual(const Event &other) const noexcept override {
            try {
                const MessageEvent &f = dynamic_cast<const MessageEvent &>(other);
                return m_callback == f.m_callback && strncmp(m_subject, other.m_subject, std::numeric_limits<uint16_t>::max()) == 0;
            } catch (const std::bad_cast &e) {
                return false;
            }
        }

        void onEvent() const {
            if (m_callback != nullptr) {
                m_callback(this, nullptr);
            }
        }

    };
}

#endif //TFDCF_LISTENER_H
