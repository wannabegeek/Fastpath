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

#ifndef FASTPATH_LISTENER_H
#define FASTPATH_LISTENER_H

#include <unordered_set>
#include <tuple>

#include "fastpath/messages/StorageTypes.h"
#include "fastpath/transport/Transport.h"
#include "fastpath/messages/MutableMessage.h"
#include "fastpath/event/Subscriber.h"
#include "fastpath/event/MessageEvent.h"

namespace fp {
    class Queue;
    class TCPTransport;
    class TransportIOEvent;

    struct TransportContainer {
        Transport *transport;
        std::unique_ptr<TransportIOEvent> event;

        TransportContainer(Transport *t, std::unique_ptr<TransportIOEvent> &&e);
    };

    class MessageListener {
    private:
        typedef std::vector<std::unique_ptr<MessageEvent>> ObserversType;
        std::unordered_map<const Transport *, ObserversType> m_observers;
        std::vector<std::unique_ptr<TransportContainer>> m_transportConnections;

        void subscribe(Transport *transport, const char *subject) noexcept;
        void unsubscribe(Transport *transport, const char *subject) noexcept;

        void handleMessage(const Transport *transport, Transport::MessageType &message);

        const bool registerTransport(Transport *transport, EventManager *eventManager);
    public:
        static MessageListener& instance(){
            static MessageListener instance;
            return instance;
        }

        status addObserver(Queue *queue, const Subscriber &subscriber, EventManager *eventManager);
        status removeObserver(Queue *queue, const Subscriber &subscriber);
    };
}

#endif //FASTPATH_LISTENER_H
