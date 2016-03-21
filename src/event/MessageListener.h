//
// Created by Tom Fewster on 29/02/2016.
//

#ifndef TFDCF_LISTENER_H
#define TFDCF_LISTENER_H

#include <unordered_set>
#include <messages/StorageTypes.h>
#include <transport/Transport.h>
#include <messages/Message.h>
#include <tuple>
#include "Subscriber.h"

namespace DCF {
    class Queue;
    class TCPTransport;
    class IOEvent;

    struct TransportContainer {
        Transport *transport;
        std::unique_ptr<IOEvent> event;

        TransportContainer(Transport *t, std::unique_ptr<IOEvent> &&e);
    };

    class MessageListener {

        typedef std::vector<const Subscriber *> ObserversType;
        std::unordered_map<const Transport *, ObserversType> m_observers;

        Transport *m_transport;

        void subscribe(Transport *transport, const char *subject) noexcept;
        void unsubscribe(Transport *transport, const char *subject) noexcept;

        void handleMessage(const Transport *transport, Message *message);

        std::unique_ptr<TransportContainer> registerTransport(Transport *transport, EventManager *eventManager);
        std::unique_ptr<TransportContainer> registerTransport(TCPTransport *transport, EventManager *eventManager);
    public:
        static MessageListener& instance(){
            static MessageListener instance;
            return instance;
        }

        ~MessageListener();

        status addObserver(Queue *queue, const Subscriber &subscriber, EventManager *eventManager);
        status removeObserver(Queue *queue, const Subscriber &subscriber);


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

//        const bool isEqual(const Event &other) const noexcept override {
//            try {
//                const MessageListener &f = dynamic_cast<const MessageListener &>(other);
//                return strcmp(m_subject, f.m_subject) == 0;
//            } catch (const std::bad_cast &e) {
//                return false;
//            }
//        }
//
//        const bool __notify(const EventType &eventType) noexcept override {
//            // no-op - we should never get here, but we require it to conform to Event interface
//            assert(true);
//            return false;
//        }
//
//        const bool __notify(const std::shared_ptr<Message> message) noexcept {
//            return m_queue->__enqueue(std::make_pair(this, std::bind(&MessageListener::dispatch, this, this, message)));
//        }
//
//        void __destroy() override {
////            m_queue->unregisterEvent(this);
//        }
    };
}

#endif //TFDCF_LISTENER_H
