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

#include "fastpath/event/InlineEventManager.h"

#include <cassert>
#include <algorithm>

#include "fastpath/utils/logger.h"
#include "fastpath/event/IOEvent.h"
#include "fastpath/event/TimerEvent.h"
#include "fastpath/event/SignalEvent.h"

namespace fp {

    InlineEventManager::InlineEventManager() noexcept : m_servicingEvents(false) {
    }

    InlineEventManager::~InlineEventManager() noexcept  {
    }

    void InlineEventManager::registerHandler(TimerEvent *event) noexcept {
        m_timerHandlerLookup.emplace(event->identifier(), event);
        m_eventLoop.add(EventPollTimerElement(event->identifier(), event->timeout()));
    }

    void InlineEventManager::registerHandler(SignalEvent *event) noexcept {
        m_signalHandlerLookup.emplace(event->signal(), event);
        m_eventLoop.add(EventPollSignalElement(event->identifier(), event->signal()));
    }

    void InlineEventManager::registerHandler(IOEvent *event) noexcept {
        if (event->fileDescriptor() <= 0) {
            ERROR_LOG("Failed to register invalid file descriptor: " << event->fileDescriptor());
            throw EventException("Invalid file descriptor");
        }

        auto it = m_ioHandlerLookup.find(event->fileDescriptor());
        if (it != m_ioHandlerLookup.end()) {
            it->second.push_back(event);
        } else {
            IOEventTable::mapped_type events;
            events.push_back(event);
            m_ioHandlerLookup.emplace(event->fileDescriptor(), std::move(events));
        }

        m_eventLoop.add(EventPollIOElement(event->fileDescriptor(), event->eventTypes()));
    }

    void InlineEventManager::updateHandler(TimerEvent *event) noexcept {
        m_eventLoop.update(EventPollTimerElement(event->identifier(), event->timeout()));
    }

    void InlineEventManager::unregisterHandler(TimerEvent *event) noexcept {
        auto it = m_timerHandlerLookup.find(event->identifier());
        if (it != m_timerHandlerLookup.end()) {
            m_timerHandlerLookup.erase(it);
            m_eventLoop.remove(EventPollTimerElement(event->identifier()));
        }
    }

    void InlineEventManager::unregisterHandler(SignalEvent *event) noexcept {
        auto it = m_signalHandlerLookup.find(event->identifier());
        if (it != m_signalHandlerLookup.end()) {
            m_signalHandlerLookup.erase(it);
            m_eventLoop.remove(EventPollSignalElement(event->identifier(), event->signal()));
        }
    }

    void InlineEventManager::unregisterHandler(IOEvent *event) noexcept {
        auto it = m_ioHandlerLookup.find(event->fileDescriptor());
        if (it != m_ioHandlerLookup.end()) {
            auto registered_events = it->second;
            auto t = std::find(registered_events.begin(), registered_events.end(), event);
            if (t != registered_events.end()) {
                // We can only remove the FD from listening if no one else is registed
                // for callbacks on it
                if (registered_events.size() == 1) {
                    m_eventLoop.remove(EventPollIOElement(event->fileDescriptor(), event->eventTypes()));
                    // upgrade read lock to write lock
                    m_ioHandlerLookup.erase(it);
                } else {
                    // upgrade read lock to write lock
                    registered_events.erase(t);
                }

                if (event->__awaitingDispatch()) {
                    assert(false); // We are removing an event which is awaiting dispatch
                }
            }
        }
    }

    void InlineEventManager::foreach_event_matching(const EventPollIOElement &event,
                                                    std::function<void(IOEvent *)> callback) const noexcept {
        auto it = m_ioHandlerLookup.find(event.identifier);
        if (it != m_ioHandlerLookup.end()) {
            m_servicingEvents = true;
            std::for_each(it->second.begin(), it->second.end(), [&](IOEvent *e) noexcept {
                if ((e->eventTypes() & event.filter) != EventType::NONE) {
                    callback(e);
                }
            });
            m_servicingEvents = false;
        }
    }

    void InlineEventManager::foreach_timer_matching(const EventPollTimerElement &event,
                                                    std::function<void(TimerEvent *)> callback) const noexcept {

        auto it = m_timerHandlerLookup.find(event.identifier);
        if (it != m_timerHandlerLookup.end()) {
            m_servicingEvents = true;
            callback(it->second);
            m_servicingEvents = false;
        }
    }

    void InlineEventManager::foreach_signal_matching(const EventPollSignalElement &event, std::function<void(SignalEvent *)> callback) const noexcept {
        auto it = m_signalHandlerLookup.find(event.identifier);
        if (it != m_signalHandlerLookup.end()) {
            m_servicingEvents = true;
            callback(it->second);
            m_servicingEvents = false;
        }
    }

    const bool InlineEventManager::haveHandlers() const noexcept {
        return !(m_timerHandlerLookup.empty() && m_ioHandlerLookup.empty());
    }

}
