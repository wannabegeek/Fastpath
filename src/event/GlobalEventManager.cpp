//
// Created by Tom Fewster on 07/03/2016.
//

#include "GlobalEventManager.h"
#include "TimerEvent.h"
#include "IOEvent.h"
#include <functional>
#include <utility>
#include <vector>
#include <utils/logger.h>

namespace DCF {

    GlobalEventManager::GlobalEventManager() {
        m_eventLoop.add(m_actionNotifier.pollElement());
    }

    GlobalEventManager::~GlobalEventManager() {
        m_eventLoop.remove(m_actionNotifier.pollElement());
    }

    void GlobalEventManager::notify(bool wait) {
        if (m_in_event_wait.load()) {
            if (wait) {
                m_actionNotifier.notify_and_wait();
            } else {
                m_actionNotifier.notify();
            }
        }
    }

    void GlobalEventManager::serviceEvent(const EventPollElement &event) {
        if (event.fd == m_actionNotifier.read_handle()) {
            m_actionNotifier.reset();
        } else {
            EventManager::serviceEvent(event);
        }
    }

    void GlobalEventManager::registerHandler(TimerEvent *event) {
        m_pendingTimerRegistrations.try_enqueue(event);
        m_lock.lock();
        m_timerHandlers.push_back(event);
        m_lock.unlock();
        this->notify(true);
    }

    void GlobalEventManager::registerHandler(IOEvent *event) {
        if (event->fileDescriptor() <= 0) {
            ERROR_LOG("Failed to register invalid file descriptor: " << event->fileDescriptor());
            throw EventException("Invalid file descriptor");
        }
        m_lock.lock();
        auto it = m_ioHandlerLookup.find(event->fileDescriptor());
        if (it != m_ioHandlerLookup.end()) {
            it->second.push_back(event);
        } else {
            IOEventTable::mapped_type events;
            events.push_back(event);
            m_ioHandlerLookup.emplace(event->fileDescriptor(), std::move(events));
        }
        m_lock.unlock();

        if (decltype(m_eventLoop)::can_add_events_async) {
            m_eventLoop.add({event->fileDescriptor(), event->eventTypes()});
        } else {
            this->notify(true);
        }
    }

    void GlobalEventManager::unregisterHandler(TimerEvent *event) {
        // read lock
        m_lock.lock_shared();
        auto it = std::find(m_timerHandlers.begin(), m_timerHandlers.end(), event);
        if (it != m_timerHandlers.end()) {
            // upgrade read lock to write lock
            m_lock.lock_upgrade();
            m_timerHandlers.erase(it);
            // unlock write lock
            m_lock.unlock_upgrade();
        }
        m_lock.unlock_shared();
    }

    void GlobalEventManager::unregisterHandler(IOEvent *event) {
        // read lock
        m_lock.lock_shared();
        auto it = m_ioHandlerLookup.find(event->fileDescriptor());
        if (it != m_ioHandlerLookup.end()) {

            auto registered_events = it->second;
            auto t = std::find(registered_events.begin(), registered_events.end(), event);
            if (t != registered_events.end()) {
                // We can only remove the FD from listening if no one else is registered
                // for callbacks on it
                if (registered_events.size() == 1) {
                    if (decltype(m_eventLoop)::can_remove_events_async) {
                        m_eventLoop.remove({event->fileDescriptor(), event->eventTypes()});
                    }
                    // upgrade read lock to write lock
                    m_lock.lock_upgrade();
                    m_ioHandlerLookup.erase(it);
                    m_lock.unlock_upgrade();
                } else {
                    m_lock.lock_upgrade();
                    registered_events.erase(t);
                    m_lock.unlock_upgrade();
                }

                if (event->__awaitingDispatch()) {
                    //assert(false); // We are removing an event which is awaiting dispatch
                }
                if (!decltype(m_eventLoop)::can_remove_events_async) {
                    this->notify(true);
                }
            }
        }
        m_lock.unlock_shared();
    }

    void GlobalEventManager::foreach_timer(std::function<void(TimerEvent *)> callback) const {
        // read lock
        m_lock.lock_shared();
        std::for_each(m_timerHandlers.begin(), m_timerHandlers.end(), std::forward<decltype(callback)>(callback));
        m_lock.unlock_shared();
    }

    void GlobalEventManager::foreach_event_matching(const EventPollElement &event,
                                                    std::function<void(IOEvent *)> callback) const {
        // read lock
        m_lock.lock_shared();
        auto it = m_ioHandlerLookup.find(event.fd);
        if (it != m_ioHandlerLookup.end()) {
            std::for_each(it->second.begin(), it->second.end(), [&](IOEvent *e) {
                if ((e->eventTypes() & event.filter) != EventType::NONE) {
                    callback(e);
                }
            });
        }
        m_lock.unlock_shared();
    }

    const bool GlobalEventManager::haveHandlers() const {
        // read lock
        m_lock.lock_shared();
        return !(m_timerHandlers.empty() && m_ioHandlerLookup.empty());
        m_lock.unlock_shared();
    }
}
