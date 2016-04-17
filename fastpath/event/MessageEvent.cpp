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

#include "MessageEvent.h"
#include "Queue.h"
#include "Subscriber.h"

namespace DCF {
    MessageEvent::MessageEvent(Queue *queue, const Subscriber *subscriber) : Event(queue), m_subscriber(subscriber) {
    }

    const bool MessageEvent::isEqual(const Event &other) const noexcept {
        if (typeid(other) == typeid(MessageEvent)) {
            const MessageEvent &f = static_cast<const MessageEvent &>(other);
            return m_subscriber == f.m_subscriber;
        }
        return false;
    }

    const bool MessageEvent::__notify(const EventType &eventType) noexcept {
        return false;
    }

    const bool MessageEvent::__notify(Transport::MessageType &message) noexcept {
        assert(m_queue != nullptr);
        this->__pushDispatch();
        return m_queue->__enqueue(QueueElement(this, std::bind(&Subscriber::__dispatch, m_subscriber, message)));
    }

    void MessageEvent::__destroy() noexcept {
        m_queue->removeSubscriber(*m_subscriber);
    }
}
