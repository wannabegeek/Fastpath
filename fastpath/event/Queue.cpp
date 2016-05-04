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

#include "fastpath/event/Queue.h"
#include "fastpath/Exception.h"
#include "fastpath/event/DataEvent.h"
#include "fastpath/event/TimerEvent.h"
#include "fastpath/event/MessageListener.h"
#include "fastpath/event/Subscriber.h"

namespace fp {

    Queue::~Queue() noexcept {
        std::for_each(m_registeredEvents.begin(), m_registeredEvents.end(), [&](auto &event) noexcept {
            DEBUG_LOG("Destroying event " << event.get());
            event->__destroy();
        });
    }

    DataEvent *Queue::registerEvent(const int fd, const EventType eventType, const std::function<void(DataEvent *, const EventType)> &callback) noexcept {
        auto result = m_registeredEvents.emplace(make_set_unique<Event>(new DataEvent(this, fd, eventType, callback)));
        assert(result.second == true);
        DataEvent *event = reinterpret_cast<DataEvent *>(result.first->get());
        EventManager *em = this->eventManager();
        if (em != nullptr) {
            em->registerHandler(event);
        } else {
            throw fp::exception("Event manager not running");
        }
        return event;
    }

    TimerEvent *Queue::registerEvent(const std::chrono::microseconds &timeout, const std::function<void(TimerEvent *)> &callback) noexcept {
        auto result = m_registeredEvents.emplace(make_set_unique<Event>(new TimerEvent(this, timeout, callback)));
        TimerEvent *event = reinterpret_cast<TimerEvent *>(result.first->get());
        EventManager *em = this->eventManager();
        if (em != nullptr) {
            em->registerHandler(event);
        } else {
            throw fp::exception("Event manager not running");
        }
        return event;
    }

    status Queue::updateEvent(TimerEvent *event) noexcept {
        EventManager *em = this->eventManager();
        if (em != nullptr) {
            em->updateHandler(event);
            return OK;
        }
        return EVM_NOTRUNNING;
    }

        status Queue::unregisterEvent(DataEvent *event) noexcept {
        // This will block any further callback to client code, which may still exist in the queue
        event->__setPendingRemoval(true);
        EventManager *em = this->eventManager();
        if (em != nullptr) {
            em->unregisterHandler(event);
            return OK;
        }
        return EVM_NOTRUNNING;
    }

    status Queue::unregisterEvent(TimerEvent *event) noexcept {
        // This will block any further callback to client code, which may still exist in the queue
        event->__setPendingRemoval(true);
        EventManager *em = this->eventManager();
        if (em != nullptr) {
            em->unregisterHandler(event);
            return OK;
        }
        return EVM_NOTRUNNING;
    }

    status Queue::addSubscriber(const Subscriber &subscriber) noexcept {
        return MessageListener::instance().addObserver(this, subscriber, this->eventManager());
    }

    status Queue::removeSubscriber(const Subscriber &subscriber) noexcept {
        return MessageListener::instance().removeObserver(this, subscriber);
    }
}