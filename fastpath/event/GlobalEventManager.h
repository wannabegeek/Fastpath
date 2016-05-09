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

#ifndef FASTPATH_GLOBALEVENTMANAGER_H
#define FASTPATH_GLOBALEVENTMANAGER_H

#include <unordered_map>
#include <vector>

#include "fastpath/utils/tfspinlock.h"
#include "fastpath/utils/rwlock.h"
#include "fastpath/event/EventManager.h"
#include "fastpath/event/notifier.h"

namespace fp {
    class Event;

    class GlobalEventManager final : public EventManager {
    private:
        using IOEventTable = std::unordered_map<int, std::vector<IOEvent *>>;
        using TimerEventTable = std::unordered_map<int, TimerEvent *>;
        using SignalEventTable = std::unordered_map<int, SignalEvent *>;

        notifier m_actionNotifier;

        IOEventTable m_ioHandlerLookup;
        TimerEventTable m_timerHandlerLookup;
        SignalEventTable m_signalHandlerLookup;

        mutable tf::rwlock m_lock;

        void foreach_event_matching(const EventPollIOElement &event, std::function<void(IOEvent *)> callback) const noexcept override;
        void foreach_timer_matching(const EventPollTimerElement &event, std::function<void(TimerEvent *)> callback) const noexcept override;
        void foreach_signal_matching(const EventPollSignalElement &event, std::function<void(SignalEvent *)> callback) const noexcept override;

        const bool haveHandlers() const noexcept override;
    public:
        GlobalEventManager();
        ~GlobalEventManager();

        void registerHandler(TimerEvent *eventRegistration) noexcept override;
        void registerHandler(IOEvent *eventRegistration) noexcept override;
        void registerHandler(SignalEvent *event) noexcept override;

        void updateHandler(TimerEvent *eventRegistration) noexcept override;

        void unregisterHandler(TimerEvent *handler) noexcept override;
        void unregisterHandler(IOEvent *handler) noexcept override;
        void unregisterHandler(SignalEvent *event) noexcept override;

        void notify(bool wait = false) noexcept override;
    };
}

#endif //FASTPATH_GLOBALEVENTMANAGER_H
