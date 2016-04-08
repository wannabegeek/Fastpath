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

#if defined HAVE_EPOLL
#   include <sys/timerfd.h>
#endif

namespace DCF {

#if defined HAVE_KEVENT
    TimerEvent::TimerEvent(Queue *queue, const std::chrono::microseconds &timeout, const std::function<void(TimerEvent *)> &callback)
            : Event(queue), m_timeout(timeout), m_callback(callback), m_identifier(++TimerEvent::s_identifier) {
    }
#elif defined HAVE_EPOLL
    TimerEvent::TimerEvent(Queue *queue, const std::chrono::microseconds &timeout, const std::function<void(TimerEvent *)> &callback)
            : Event(queue), m_timeout(timeout), m_callback(callback), m_identifier(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK)) {

            struct timespec t;
            t.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(timeout).count();
            t.tv_nsec = static_cast<decltype(t.tv_nsec)>(std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count() - (t.tv_sec * 1000000000));

            struct itimerspec interval;
            interval.it_interval = t;
            interval.it_value = t;

            if (timerfd_settime(m_identifier, 0, &interval, NULL) == -1) {
                throw fp::exception("Failed to create timer: "); // << strerror(errno));
            }
    }
#endif

    TimerEvent::TimerEvent(TimerEvent &&other) noexcept : Event(std::move(other)), m_timeout(std::move(other.m_timeout)), m_callback(std::move(other.m_callback)), m_identifier(other.m_identifier) {
    }

    void TimerEvent::dispatch(TimerEvent *event) {
        this->__popDispatch();
        if (tf::likely(!m_pendingRemoval)) {
            m_callback(event);
        }
    }

    void TimerEvent::reset() {
#if defined HAVE_KEVENT
        m_queue->updateEvent(this);
#elif defined HAVE_EPOLL
        struct timespec t;
        t.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(m_timeout).count();
        t.tv_nsec = static_cast<decltype(t.tv_nsec)>(std::chrono::duration_cast<std::chrono::nanoseconds>(m_timeout).count() - (t.tv_sec * 1000000000));

        struct itimerspec interval;
        interval.it_interval = t;
        interval.it_value = t;

        if (timerfd_settime(m_identifier, 0, &interval, NULL) == -1) {
            throw fp::exception("Failed to create timer: ");// << strerror(errno));
        }
#endif
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

#if defined HAVE_KEVENT
    std::atomic<int> TimerEvent::s_identifier = ATOMIC_VAR_INIT(0);
#endif
}
