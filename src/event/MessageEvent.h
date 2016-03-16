//
// Created by Tom Fewster on 29/02/2016.
//

#ifndef TFDCF_LISTENER_H
#define TFDCF_LISTENER_H

#include <messages/StorageTypes.h>
#include <transport/Transport.h>
#include <messages/Message.h>
#include "Event.h"

namespace DCF {
    class MessageEvent : public Event {

        char m_subject[std::numeric_limits<uint16_t>::max()];
        Transport *m_transport;
        std::function<void(const MessageEvent *, const Message *)> m_callback;

        void dispatch(MessageEvent *event, const Message *msg) {
            this->__setAwaitingDispatch(false);
            if (m_active) {
                m_callback(event, msg);
            }
        }

        inline void subscribe() noexcept {
            Message msg;
            msg.setSubject("_FP.REGISTER.OBSERVER");
            msg.addDataField("subject", m_subject);
            msg.addScalarField("id", reinterpret_cast<uint64_t>(this));
            m_transport->sendMessage(msg);
        }

        inline void unsubscribe() noexcept {
            Message msg;
            msg.setSubject("_FP.UNREGISTER.OBSERVER");
            msg.addDataField("subject", m_subject);
            msg.addScalarField("id", reinterpret_cast<uint64_t>(this));
            m_transport->sendMessage(msg);
        }

    public:
        MessageEvent(Queue *queue, Transport *transport, const char *subject, const std::function<void(const MessageEvent *, const Message *)> &callback) : Event(queue), m_transport(transport), m_callback(callback) {
            const size_t subject_length = strlen(subject);
            std::copy(subject, &subject[subject_length], m_subject);
            this->subscribe();
        };

        ~MessageEvent() {
            this->unsubscribe();
        }

        void registerEvent(Queue *queue, Transport *transport, const char *subject, const std::function<void(const MessageEvent *, const Message *)> &callback) {
            setQueue(queue);
            m_callback = callback;
            const size_t subject_length = strlen(subject);
            std::copy(subject, &subject[subject_length], m_subject);

            this->subscribe();
            m_active = true;
        }

        void unregisterEvent() {
            if (m_active && m_isRegistered.load()) {
                m_active = false;
                this->unsubscribe();
            }
        }

        const bool isEqual(const Event &other) const noexcept override {
            try {
                const MessageEvent &f = dynamic_cast<const MessageEvent &>(other);
                return strcmp(m_subject, f.m_subject) == 0;
            } catch (const std::bad_cast &e) {
                return false;
            }
        }

        const bool __notify(const EventType &eventType) noexcept override {
//            std::function<void ()> dispatcher = std::bind(&MessageEvent::dispatch, this, static_cast<DCF::MessageEvent *>(this), eventType);
//            return m_queue->__enqueue(dispatcher);
            return false;
        };
    };
}

#endif //TFDCF_LISTENER_H
