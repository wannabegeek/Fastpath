//
// Created by Tom Fewster on 07/03/2016.
//

#include <utils/logger.h>
#include "InlineEventManager.h"
#include "IOEvent.h"
#include "TimerEvent.h"


namespace DCF {

    InlineEventManager::InlineEventManager() : m_servicingEvents(false), m_servicingTimers(false), m_pendingFileDescriptorRegistrationEvents(false), m_pendingTimerRegistrationEvents(false) {
    }

    InlineEventManager::~InlineEventManager() {
    }

    void InlineEventManager::processPendingRegistrations() {
        if (m_pendingTimerRegistrationEvents) {
            m_timerHandlers = m_pendingTimerHandlers;
            m_pendingTimerRegistrationEvents = false;
        }
    }

    void InlineEventManager::registerHandler(TimerEvent &event) {
        if (m_servicingTimers) {
            m_pendingTimerRegistrationEvents = true;
            m_pendingTimerHandlers.push_back(&event);
        } else {
            m_timerHandlers.push_back(&event);
            m_pendingTimerHandlers.push_back(&event);
        }
        // we can do this for the InlineEventManager since we must be out of the event loop to get here!
        event.__setIsRegistered(true);
    }

    void InlineEventManager::registerHandler(IOEvent &event) {
        if (event.fileDescriptor() <= 0) {
            ERROR_LOG("Failed to register invalid file descriptor: " << event.fileDescriptor());
            throw EventException("Invalid file descriptor");
        }

        auto it = m_ioHandlerLookup.find(event.fileDescriptor());
        if (it != m_ioHandlerLookup.end()) {
            it->second.push_back(&event);
        } else {
            IOEventTable::mapped_type events;
            events.push_back(&event);
            m_ioHandlerLookup.emplace(event.fileDescriptor(), std::move(events));
        }

        m_eventLoop.add({event.fileDescriptor(), event.eventTypes()});
        event.__setIsRegistered(true);
    }

    void InlineEventManager::unregisterHandler(TimerEvent &handler) {
        auto it = std::find(m_pendingTimerHandlers.begin(), m_pendingTimerHandlers.end(), &handler);
        if (it != m_pendingTimerHandlers.end()) {
            m_pendingTimerHandlers.erase(it);
        }
        if (m_servicingTimers) {
            m_pendingTimerRegistrationEvents = true;
        } else {
            auto it = std::find(m_timerHandlers.begin(), m_timerHandlers.end(), &handler);
            if (it != m_timerHandlers.end()) {
                m_timerHandlers.erase(it);
            }
        }

        handler.__setIsRegistered(false);
    }

    void InlineEventManager::unregisterHandler(IOEvent &event) {
        auto it = m_ioHandlerLookup.find(event.fileDescriptor());
        if (it != m_ioHandlerLookup.end()) {
            auto registered_events = it->second;
            auto t = std::find(registered_events.begin(), registered_events.end(), &event);
            if (t != registered_events.end()) {
                // We can only remove the FD from listening if no one else is registed
                // for callbacks on it
                if (registered_events.size() == 1) {
                    if (decltype(m_eventLoop)::can_add_events_async) {
                        m_eventLoop.remove({event.fileDescriptor(), event.eventTypes()});
                    }
                    // upgrade read lock to write lock
                    m_ioHandlerLookup.erase(it);
                } else {
                    // upgrade read lock to write lock
                    registered_events.erase(t);
                }

                if (event.__awaitingDispatch()) {
                    assert(false); // We are removing an event which is awaiting dispatch
                }
                event.__setIsRegistered(false);
            }
        }
    }

    void InlineEventManager::foreach_timer(std::function<void(TimerEvent *)> callback) const {
        m_servicingTimers = true;
        std::for_each(m_timerHandlers.begin(), m_timerHandlers.end(), std::forward<decltype(callback)>(callback));
        m_servicingTimers = false;
    }

    void InlineEventManager::foreach_event_matching(const EventPollElement &event,
                                                    std::function<void(IOEvent *)> callback) const {
        auto it = m_ioHandlerLookup.find(event.fd);
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

    const bool InlineEventManager::haveHandlers() const {
        return !(m_timerHandlers.empty() && m_ioHandlerLookup.empty());
    }
}