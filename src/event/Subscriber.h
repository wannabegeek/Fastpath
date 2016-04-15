/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef TFDCF_SUBSCRIBER_H
#define TFDCF_SUBSCRIBER_H

#include <functional>

#include "router/subscription.h"
#include "transport/Transport.h"

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
