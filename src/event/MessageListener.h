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
    class MessageListener : public Event {

        char m_subject[std::numeric_limits<uint16_t>::max()];
        Transport *m_transport;
        std::function<void(const MessageListener *, const Message *)> m_callback;

        void dispatch(MessageListener *event, const std::shared_ptr<Message> msg) {
            this->__popDispatch();
            if (!m_pendingRemoval) {
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
        MessageListener(Queue *queue, Transport *transport, const char *subject, const std::function<void(const MessageListener *, const Message *)> &callback) : Event(queue), m_transport(transport), m_callback(callback) {
            const size_t subject_length = strlen(subject);
            std::copy(subject, &subject[subject_length], m_subject);
            this->subscribe();
        };

        ~MessageListener() {
                this->unsubscribe();
//                    m_queue->unregisterEvent(*this);
        }

//        status create(Transport *transport, const char *subject, const std::function<void(const MessageListener *, const Message *)> &callback) {
//            if (!m_active) {
//                const size_t subject_length = strlen(subject);
//                std::copy(subject, &subject[subject_length], m_subject);
//
//                this->subscribe();
//                m_callback = callback;
//                m_active = true;
//            } else {
//                return CANNOT_CREATE;
//            }
//            return OK;
//        }
//
//        status destroy() {
//            if (m_active) {
//                m_active = false;
//                if (m_isRegistered.load()) {
//                    this->unsubscribe();
////                    m_queue->unregisterEvent(*this);
//                }
//            } else {
//                return CANNOT_DESTROY;
//            }
//            return OK;
//        }

        const bool isEqual(const Event &other) const noexcept override {
            try {
                const MessageListener &f = dynamic_cast<const MessageListener &>(other);
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
            return m_queue->__enqueue(std::make_pair(this, std::bind(&MessageListener::dispatch, this, this, message)));
        }

        void __destroy() override {
//            m_queue->unregisterEvent(this);
        }
    };
}

#endif //TFDCF_LISTENER_H
