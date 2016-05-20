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

#include "fastpath/event/SignalEvent.h"
#include "fastpath/event/Queue.h"
#include "fastpath/config.h"

#include <cassert>
#include <signal.h>

#if defined HAVE_EPOLL
#   include <sys/signalfd.h>
#endif

namespace fp {

#if defined HAVE_KQUEUE
    SignalEvent::SignalEvent(Queue *queue, const int signal, const std::function<void(SignalEvent *, int)> &callback)
            : Event(queue), m_callback(callback), m_signal(signal) {
        ::signal(signal, SIG_IGN);
    }
#elif defined HAVE_EPOLL
    SignalEvent::SignalEvent(Queue *queue, const int signal, const std::function<void(SignalEvent *, int)> &callback)
            : Event(queue), m_callback(callback), m_signal(signal) {

        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, signal);

        m_identifier = signalfd(-1, &mask, SFD_NONBLOCK);
        if (m_identifier == -1) {
            throw fp::exception("Failed to timerfd_create: "); // << strerror(errno));
        }

        if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
            ERROR_LOG("Failed to block default signal delivery");
        }
        if (pthread_sigmask(SIG_BLOCK, &mask, NULL) == -1) {
            ERROR_LOG("Failed to block default signal delivery");
        }
    }
#endif

    SignalEvent::SignalEvent(SignalEvent &&other) noexcept : Event(std::move(other)), m_identifier(other.m_identifier), m_callback(std::move(other.m_callback)), m_signal(other.m_signal) {
    }

    void SignalEvent::dispatch(SignalEvent *event) noexcept {
        this->__popDispatch();
        if (tf::likely(!m_pendingRemoval)) {
            m_callback(event, m_signal);
        }
    }

    const bool SignalEvent::isEqual(const Event &other) const noexcept {
        if (typeid(other) == typeid(SignalEvent)) {
            const SignalEvent &f = static_cast<const SignalEvent &>(other);
            return m_signal == f.m_signal;
        }
        return false;
    }

    const bool SignalEvent::__notify(const EventType &eventType) noexcept {
        assert(m_queue != nullptr);
        this->__pushDispatch();
        return m_queue->__enqueue(QueueElement(this, std::bind(&SignalEvent::dispatch, this, this)));
    }

    void SignalEvent::__destroy() noexcept {
        m_queue->unregisterEvent(this);
    }
}
