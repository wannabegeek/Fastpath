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

#include "fastpath/event/GlobalEventManager.h"

#include <functional>
#include <utility>
#include <vector>
#include <shared_mutex>

#include "fastpath/event/TimerEvent.h"
#include "fastpath/event/IOEvent.h"
#include "fastpath/utils/logger.h"

namespace fp {

    GlobalEventManager::GlobalEventManager() {
        m_eventLoop.add(m_actionNotifier.pollElement());
    }

    GlobalEventManager::~GlobalEventManager() {
        m_eventLoop.remove(m_actionNotifier.pollElement());
    }

    void GlobalEventManager::notify(bool wait) noexcept {
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

    void GlobalEventManager::registerHandler(TimerEvent *event) noexcept {
        m_lock.lock();
        m_timerHandlerLookup.emplace(event->identifer(), event);
        m_lock.unlock();
        m_eventLoop.add({event->identifer(), event->timeout()});
    }

    void GlobalEventManager::registerHandler(IOEvent *event) noexcept {
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

        m_eventLoop.add({event->fileDescriptor(), event->eventTypes()});
    }

    void GlobalEventManager::updateHandler(TimerEvent *eventRegistration) noexcept {

    }

    void GlobalEventManager::unregisterHandler(TimerEvent *event) noexcept {
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

    void GlobalEventManager::unregisterHandler(IOEvent *event) noexcept {
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
                                                    std::function<void(IOEvent *)> callback) const noexcept {
        // read lock
        m_lock.lock_shared();
        auto it = m_ioHandlerLookup.find(event.identifier);
        if (it != m_ioHandlerLookup.end()) {
            std::for_each(it->second.begin(), it->second.end(), [&](IOEvent *e) noexcept {
                if ((e->eventTypes() & event.filter) != EventType::NONE) {
                    callback(e);
                }
            });
        }
        m_lock.unlock_shared();
    }

    void GlobalEventManager::foreach_timer_matching(const EventPollTimerElement &event,
                                                    std::function<void(TimerEvent *)> callback) const noexcept {
        // read lock
        m_lock.lock_shared();
        auto it = m_timerHandlerLookup.find(event.identifier);
        if (it != m_timerHandlerLookup.end()) {
            callback(it->second);
        }
        m_lock.unlock_shared();
    }

    const bool GlobalEventManager::haveHandlers() const noexcept {
        // read lock
        std::shared_lock<tf::rwlock> lock(m_lock);
        return !(m_timerHandlerLookup.empty() && m_ioHandlerLookup.empty());
    }
}
