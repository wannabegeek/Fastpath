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

#ifndef FASTPATH_MESSAGEEVENT_H
#define FASTPATH_MESSAGEEVENT_H

#include "fastpath/event/Event.h"
#include "fastpath/transport/Transport.h"

namespace fp {
    class Subscriber;

    class MessageEvent : public Event {
    private:
        const Subscriber *m_subscriber;

        virtual const bool isEqual(const Event &other) const noexcept override;
    public:
        MessageEvent(Queue *queue, const Subscriber *subscriber);

        virtual const bool __notify(const EventType &eventType) noexcept override;
        const bool __notify(Transport::MessageType &message) noexcept;
        virtual void __destroy() noexcept override;

        const Subscriber *subscriber() const noexcept { return m_subscriber; }
    };
}

#endif //FASTPATH_MESSAGEEVENT_H
