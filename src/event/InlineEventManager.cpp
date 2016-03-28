//
// Created by Tom Fewster on 07/03/2016.
//

#include <utils/logger.h>
#include "InlineEventManager.h"
#include "IOEvent.h"
#include "TimerEvent.h"
#include <cassert>

namespace DCF {

    InlineEventManager::InlineEventManager() : m_servicingEvents(false), m_servicingTimers(false) {
    }

    InlineEventManager::~InlineEventManager() {
    }

    void InlineEventManager::registerHandler(TimerEvent *event) {
        m_timerHandlerLookup.emplace(event->identifer(), event);
        m_eventLoop.add({event->identifer(), event->timeout()});
    }

    void InlineEventManager::registerHandler(IOEvent *event) {
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

        m_eventLoop.add({event->fileDescriptor(), event->eventTypes()});
    }

    void InlineEventManager::updateHandler(TimerEvent *event) {
        m_eventLoop.update({event->identifer(), event->timeout()});
    }

    void InlineEventManager::unregisterHandler(TimerEvent *event) {
        auto it = m_timerHandlerLookup.find(event->identifer());
        if (it != m_timerHandlerLookup.end()) {
            m_timerHandlerLookup.erase(it);
            m_eventLoop.remove({event->identifer()});
        }
    }

    void InlineEventManager::unregisterHandler(IOEvent *event) {
        auto it = m_ioHandlerLookup.find(event->fileDescriptor());
        if (it != m_ioHandlerLookup.end()) {
            auto registered_events = it->second;
            auto t = std::find(registered_events.begin(), registered_events.end(), event);
            if (t != registered_events.end()) {
                // We can only remove the FD from listening if no one else is registed
                // for callbacks on it
                if (registered_events.size() == 1) {
                    if (decltype(m_eventLoop)::can_add_events_async) {
                        m_eventLoop.remove({event->fileDescriptor(), event->eventTypes()});
                    }
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
                                                    std::function<void(IOEvent *)> callback) const {
        auto it = m_ioHandlerLookup.find(event.identifier);
        if (it != m_ioHandlerLookup.end()) {
            m_servicingEvents = true;
            std::for_each(it->second.begin(), it->second.end(), [&](IOEvent *e) {
                if ((e->eventTypes() & event.filter) != EventType::NONE) {
                    callback(e);
                }
            });
            m_servicingEvents = false;
        }
    }

    void InlineEventManager::foreach_timer_matching(const EventPollTimerElement &event,
                                                    std::function<void(TimerEvent *)> callback) const {

        auto it = m_timerHandlerLookup.find(event.identifier);
        if (it != m_timerHandlerLookup.end()) {
            m_servicingEvents = true;
            callback(it->second);
            m_servicingEvents = false;
        }
    }

    const bool InlineEventManager::haveHandlers() const {
        return !(m_timerHandlerLookup.empty() && m_ioHandlerLookup.empty());
    }

}