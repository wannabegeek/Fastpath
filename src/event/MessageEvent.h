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

        void dispatch(MessageEvent *event, const std::shared_ptr<Message> msg) {
            this->__setAwaitingDispatch(false);
            if (m_active) {
                m_callback(event, msg.get());
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
        MessageEvent(Transport *transport, const char *subject, const std::function<void(const MessageEvent *, const Message *)> &callback) : m_transport(transport), m_callback(callback) {
            const size_t subject_length = strlen(subject);
            std::copy(subject, &subject[subject_length], m_subject);
            this->subscribe();
            m_active = true;
        };

        ~MessageEvent() {
            this->destroy();
        }

        status create(Transport *transport, const char *subject, const std::function<void(const MessageEvent *, const Message *)> &callback) {
            if (!m_active) {
                const size_t subject_length = strlen(subject);
                std::copy(subject, &subject[subject_length], m_subject);

                this->subscribe();
                m_callback = callback;
                m_active = true;
            } else {
                return ALREADY_ACTIVE;
            }
            return OK;
        }

        status destroy() {
            if (m_active) {
                m_active = false;
                if (m_isRegistered.load()) {
                    this->unsubscribe();
//                    m_queue->__unregisterEvent(*this);
                }
            } else {
                return CANNOT_DESTROY;
            }
            return OK;
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
            // no-op - we should never get here, but we require it to conform to Event interface
            assert(true);
            return false;
        }

        const bool __notify(const std::shared_ptr<Message> message) noexcept {
            std::function<void ()> dispatcher = std::bind(&MessageEvent::dispatch, this, this, message);
            return m_queue->__enqueue(dispatcher);
        };
    };
}

#endif //TFDCF_LISTENER_H
