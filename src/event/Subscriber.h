//
// Created by Tom Fewster on 20/03/2016.
//

#ifndef TFDCF_SUBSCRIBER_H
#define TFDCF_SUBSCRIBER_H

#include <functional>
#include <messages/Message.h>

namespace DCF {
    class Transport;
    class Message;

    class Subscriber {
    private:
        Transport *m_transport;
        std::unique_ptr<char[]> m_subject;
        std::function<void(Subscriber *, Message *)> m_callback;

    public:
        Subscriber(Transport *transport, const char *subject, const std::function<void(Subscriber *, Message *)> &callback) : m_transport(transport), m_callback(callback) {
            // TODO: we need to copy the string here
            const size_t len = strlen(subject);
            m_subject = std::unique_ptr<char[]>(new char[len + 1]);
            std::copy_n(subject, len + 1, m_subject.get());
        }

        ~Subscriber();

        Transport *transport() const noexcept { return m_transport; }
        const char *subject() const noexcept { return m_subject.get(); }
    };
}

#endif //TFDCF_SUBSCRIBER_H
