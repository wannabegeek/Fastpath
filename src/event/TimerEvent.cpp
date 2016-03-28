/***************************************************************************
                          Timer.cpp
                          -------------------
    copyright            : (C) 2004 by Tom Fewster
    email                : tom@wannabegeek.com
    version              : $Revision: 1.19 $
    date                 : $Date: 2004/03/04 08:44:13 $

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

#include "TimerEvent.h"
#include "Queue.h"

namespace DCF {

    TimerEvent::TimerEvent(Queue *queue, const std::chrono::milliseconds &timeout, const std::function<void(TimerEvent *)> &callback)
            : Event(queue), m_timeoutState(TIMEOUTSTATE_START), m_timeout(timeout), m_timeLeft(timeout), m_callback(callback) {
    }

    TimerEvent::TimerEvent(TimerEvent &&other) : Event(std::move(other)), m_timeout(std::move(other.m_timeout)), m_callback(std::move(other.m_callback)) {
    }

    void TimerEvent::dispatch(TimerEvent *event) {
        this->__popDispatch();
        if (tf::likely(!m_pendingRemoval)) {
            m_callback(event);
        }
    }

    void TimerEvent::reset() {
        m_timeLeft = m_timeout;
        m_queue->__notifyEventManager();
    }

    void TimerEvent::setTimeout(const std::chrono::microseconds &timeout) {
        m_timeout = timeout;
        this->reset();
    }

    const bool TimerEvent::isEqual(const Event &other) const noexcept {
        try {
            const TimerEvent &f = dynamic_cast<const TimerEvent &>(other);
            return m_timeout == f.m_timeout;
        } catch (const std::bad_cast &e) {
            return false;
        }
    }

    const bool TimerEvent::__notify(const EventType &eventType) noexcept {
        assert(m_queue != nullptr);
        this->__pushDispatch();
        return m_queue->__enqueue(QueueElement(this, std::bind(&TimerEvent::dispatch, this, this)));
    }

    void TimerEvent::__destroy() {
        m_queue->unregisterEvent(this);
    }
}
