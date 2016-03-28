//
// Created by Tom Fewster on 07/03/2016.
//

#include "GlobalEventManager.h"
#include "TimerEvent.h"
#include "IOEvent.h"
#include <functional>
#include <utility>
#include <vector>
#include <shared_mutex>
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

//    void GlobalEventManager::serviceEvent(const EventPollIOElement &event) {
//        if (event.identifier == m_actionNotifier.read_handle()) {
//            m_actionNotifier.reset();
//        } else {
//            EventManager::serviceEvent(event);
//        }
//    }

    void GlobalEventManager::registerHandler(TimerEvent *event) {
        m_lock.lock();
        m_timerHandlerLookup.emplace(event->identifer(), event);
        m_lock.unlock();
        m_eventLoop.add({event->identifer(), event->timeout()});
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

    void GlobalEventManager::updateHandler(TimerEvent *eventRegistration) {

    }

    void GlobalEventManager::unregisterHandler(TimerEvent *event) {
        // read lock
        m_lock.lock_shared();
        auto it = m_timerHandlerLookup.find(event->identifer());
        if (it != m_timerHandlerLookup.end()) {
            m_eventLoop.remove({event->identifer()});
            // upgrade read lock to write lock
            m_lock.lock_upgrade();
            m_timerHandlerLookup.erase(it);
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
                    m_eventLoop.remove({event->fileDescriptor(), event->eventTypes()});
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
            }
        }
        m_lock.unlock_shared();
    }

    void GlobalEventManager::foreach_event_matching(const EventPollIOElement &event,
                                                    std::function<void(IOEvent *)> callback) const {
        // read lock
        m_lock.lock_shared();
        auto it = m_ioHandlerLookup.find(event.identifier);
        if (it != m_ioHandlerLookup.end()) {
            std::for_each(it->second.begin(), it->second.end(), [&](IOEvent *e) {
                if ((e->eventTypes() & event.filter) != EventType::NONE) {
                    callback(e);
                }
            });
        }
        m_lock.unlock_shared();
    }

    void GlobalEventManager::foreach_timer_matching(const EventPollTimerElement &event,
                                                    std::function<void(TimerEvent *)> callback) const {
        // read lock
        m_lock.lock_shared();
        auto it = m_timerHandlerLookup.find(event.identifier);
        if (it != m_timerHandlerLookup.end()) {
            callback(it->second);
        }
        m_lock.unlock_shared();
    }

    const bool GlobalEventManager::haveHandlers() const {
        // read lock
        std::shared_lock<tf::rwlock> lock(m_lock);
        return !(m_timerHandlerLookup.empty() && m_ioHandlerLookup.empty());
    }
}
