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

#include "fastpath/event/EventManager.h"

#include "fastpath/config.h"

#include <thread>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <array>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <unistd.h>
#include <sys/time.h>
#include <sys/fcntl.h>

#ifdef HAVE_SYS_EVENTFD_H
#	include <sys/eventfd.h>
#endif


#include "fastpath/event/PollManager.h"
#include "fastpath/utils/optimize.h"
#include "fastpath/event/EventType.h"
#include "fastpath/event/TimerEvent.h"
#include "fastpath/event/IOEvent.h"
#include "fastpath/utils/logger.h"

namespace DCF {
    EventManager::EventManager() : m_ioCallback(std::bind(&EventManager::serviceIOEvent, this, std::placeholders::_1)), m_timerCallback(std::bind(&EventManager::serviceTimerEvent, this, std::placeholders::_1)) {}

	bool isFileDescriptorValid(int fd) {
		return (!(::fcntl(fd, F_GETFL) == -1 && errno != EBADF));
	}

    void EventManager::waitForEvent() noexcept {
		int result = 0;
        m_in_event_wait.store(true); // not sure if this should be before the setTimeout() or below - lets be safe
        result = m_eventLoop.run(m_ioCallback, m_timerCallback);
        m_in_event_wait.store(false);
        if (tf::unlikely(result == -1)) {
            ERROR_LOG("We have an error: [errno: " << strerror(errno) << "(" << errno << ")]");
        }
	}

	void EventManager::serviceIOEvent(const EventPollIOElement &event) noexcept {
        foreach_event_matching(event, [&](IOEvent *handler) noexcept {
            if (!handler->__awaitingDispatch()) {
                handler->__notify(static_cast<EventType>(handler->eventTypes() & event.filter));
            }
        });
	}

    void EventManager::serviceTimerEvent(const EventPollTimerElement &event) noexcept {
        foreach_timer_matching(event, [&](TimerEvent *handler) noexcept {
            handler->__notify(EventType::NONE);
        });
    }
}
