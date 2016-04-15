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

#ifndef __TFFIXEngine__TFSessionEventManager__
#define __TFFIXEngine__TFSessionEventManager__

#include "PollManager.h"

#include <limits>
#include <functional>

namespace DCF {
    class TimerEvent;
    class IOEvent;
    
	class EventManager {
    public:
        static constexpr const size_t maxEvents = std::numeric_limits<uint16_t>::max();

	private:
		std::function<void(EventPollIOElement &&)> m_ioCallback;
		std::function<void(EventPollTimerElement &&)> m_timerCallback;
    protected:
        EventPoll<maxEvents> m_eventLoop;

        std::atomic<bool> m_in_event_wait = ATOMIC_VAR_INIT(false);

        virtual void serviceIOEvent(const EventPollIOElement &event) noexcept;
        virtual void serviceTimerEvent(const EventPollTimerElement &event) noexcept;

        virtual void foreach_event_matching(const EventPollIOElement &event, std::function<void(IOEvent *)> callback) const noexcept = 0;
        virtual void foreach_timer_matching(const EventPollTimerElement &event, std::function<void(TimerEvent *)> callback) const noexcept = 0;

        virtual const bool haveHandlers() const noexcept = 0;

    public:
        EventManager();

        EventManager(const EventManager &) = delete;
        EventManager &operator=(const EventManager &) = delete;

        virtual ~EventManager() {};

        virtual void registerHandler(TimerEvent *eventRegistration) noexcept = 0;
		virtual void registerHandler(IOEvent *eventRegistration) noexcept = 0;
        virtual void updateHandler(TimerEvent *eventRegistration) noexcept = 0;
		virtual void unregisterHandler(TimerEvent *handler) noexcept = 0;
		virtual void unregisterHandler(IOEvent *handler) noexcept = 0;

		virtual void notify(bool wait = false) noexcept = 0;

		void waitForEvent() noexcept;
	};

}
#endif /* defined(__TFFIXEngine__TFSessionEventManager__) */
