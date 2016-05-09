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

#ifndef FASTPATH_INLINEEVENTMANAGER_H
#define FASTPATH_INLINEEVENTMANAGER_H

#include <unordered_map>
#include <vector>

#include "fastpath/event/EventManager.h"

namespace fp {

    class InlineEventManager final : public EventManager {
    private:
        using IOEventTable = std::unordered_map<int, std::vector<IOEvent *>>;
        using TimerEventTable = std::unordered_map<int, TimerEvent *>;
        using SignalEventTable = std::unordered_map<int, SignalEvent *>;

        IOEventTable m_ioHandlerLookup;
        TimerEventTable m_timerHandlerLookup;
        SignalEventTable m_signalHandlerLookup;

        mutable bool m_servicingEvents;
//        mutable bool m_servicingTimers;

        void foreach_event_matching(const EventPollIOElement &event, std::function<void(IOEvent *)> callback) const noexcept override;
        void foreach_timer_matching(const EventPollTimerElement &event, std::function<void(TimerEvent *)> callback) const noexcept override;
        void foreach_signal_matching(const EventPollSignalElement &event, std::function<void(SignalEvent *)> callback) const noexcept override;

        const bool haveHandlers() const noexcept override;
    public:
        InlineEventManager() noexcept;
//        InlineEventManager(InlineEventManager &&other) noexcept;

        ~InlineEventManager() noexcept;

        void registerHandler(TimerEvent *event) noexcept override;
        void registerHandler(IOEvent *event) noexcept override;
        void registerHandler(SignalEvent *event) noexcept override;

        void updateHandler(TimerEvent *event) noexcept override;

        void unregisterHandler(TimerEvent *event) noexcept override;
        void unregisterHandler(IOEvent *event) noexcept override;
        void unregisterHandler(SignalEvent *event) noexcept override;

        void notify(bool wait = false) noexcept override {}
    };
}

#endif //FASTPATH_INLINEEVENTMANAGER_H
