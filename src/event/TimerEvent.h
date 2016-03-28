/***************************************************************************
                          Timer.h
                          -------------------
    copyright            : (C) 2004 by Tom Fewster
    email                : tom@wannabegeek.com
    version              : $Revision: 1.14 $
    date                 : $Date: 2004/02/10 14:24:36 $

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

#ifndef TIMER_H
#define TIMER_H

class TimerCallback;
class DCQueue;

#include <chrono>
#include "Event.h"
#include "status.h"

namespace  DCF {

    class Queue;

	class TimerEvent final : public Event {
        friend class EventManager;

    private:
#if defined HAVE_KEVENT
        static std::atomic<int> s_identifier;
#elif defined HAVE_EPOLL
#endif
		std::chrono::microseconds m_timeout;

        std::function<void(TimerEvent *)> m_callback;
        const int m_identifier;

        void dispatch(TimerEvent *event);

    public:
		TimerEvent(Queue *queue, const std::chrono::microseconds &timeout, const std::function<void(TimerEvent *)> &callback);
		TimerEvent(TimerEvent &&other);

        void reset();
		void setTimeout(const std::chrono::microseconds &timeout);

        const int identifer() const noexcept { return m_identifier; }
        const std::chrono::microseconds &timeout() const noexcept { return m_timeout; }

		const bool isEqual(const Event &other) const noexcept override;
        const bool __notify(const EventType &eventType) noexcept override;
        void __destroy() override;
    };
}

#endif
