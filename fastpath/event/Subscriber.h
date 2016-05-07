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

#ifndef FASTPATH_SUBSCRIBER_H
#define FASTPATH_SUBSCRIBER_H

#include <functional>

#include "fastpath/messages/subscription.h"
#include "fastpath/transport/Transport.h"
#include "fastpath/messages/Message.h"

namespace fp {
    class Transport;
    class Message;

    class Subscriber {
    private:
        Transport *m_transport;
        char m_subject[Message::max_subject_length];
        const std::function<void(const Subscriber *, Message *)> m_callback;
        subscription<> m_subscription;

    public:
        Subscriber(std::unique_ptr<Transport> &transport, const char *subject, const std::function<void(const Subscriber *, Message *)> &callback) : Subscriber(transport.get(), subject, callback) { }

        Subscriber(Transport *transport, const char *subject, const std::function<void(const Subscriber *, Message *)> &callback) : m_transport(transport), m_callback(callback), m_subscription(subject) {
            ::strncpy(m_subject, subject, Message::max_subject_length);
        }

        ~Subscriber() noexcept = default;

        Transport *transport() const noexcept { return m_transport; }
        const char *subject() const noexcept { return m_subject; }

        bool is_interested(const ::fp::subject<> subject) const;

        void __dispatch(Transport::MessageType &message) const;
    };
}

#endif //FASTPATH_SUBSCRIBER_H
