//
// Created by Tom Fewster on 20/03/2016.
//

#ifndef TFDCF_SUBSCRIBER_H
#define TFDCF_SUBSCRIBER_H

#include <functional>
#include <messages/Message.h>
#include <router/subscription.h>

namespace DCF {
    class Transport;
    class Message;

    class Subscriber {
    private:
        Transport *m_transport;
        std::unique_ptr<char[]> m_subject;
        const std::function<void(const Subscriber *, Message *)> m_callback;
        fp::subscription<> m_subscription;

    public:
        Subscriber(std::unique_ptr<Transport> &transport, const char *subject, const std::function<void(const Subscriber *, Message *)> &callback) : Subscriber(transport.get(), subject, callback) { }

        Subscriber(Transport *transport, const char *subject, const std::function<void(const Subscriber *, Message *)> &callback) : m_transport(transport), m_callback(callback), m_subscription(subject) {
            // TODO: we need to copy the string here
            const size_t len = strlen(subject);
            m_subject = std::unique_ptr<char[]>(new char[len + 1]);
            std::copy_n(subject, len + 1, m_subject.get());
        }

        ~Subscriber() {};

        Transport *transport() const noexcept { return m_transport; }
        const char *subject() const noexcept { return m_subject.get(); }

        bool is_interested(const fp::subject<> subject) const;

        void __dispatch(Transport::MessageType &message) const;
    };
}

#endif //TFDCF_SUBSCRIBER_H
