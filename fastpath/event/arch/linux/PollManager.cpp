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

#include "fastpath/event/PollManager.h"
#include "fastpath/event/EventType.h"
#include "fastpath/utils/logger.h"

#include <thread>
#include <iostream>
#include <cstring>
#include <sys/signalfd.h>
#include <sys/epoll.h>

namespace fp {

    static const uint64_t TimerIdentifier = 1l << 33;
    static const uint64_t SignalIdentifier = 2l << 33;

    EventPoll::EventPoll() {
        m_fd = epoll_create1(0);
        if (m_fd == -1) {
            ERROR_LOG("Failed to create epoll fd: " << strerror(errno));
            return;
        }
    }

    bool EventPoll::add(const EventPollTimerElement &event) noexcept {
        struct epoll_event ev;
        memset(&ev, 0, sizeof(struct epoll_event));

        ev.events = EPOLLIN;
        ev.data.u64 = event.identifier | TimerIdentifier;

        if (epoll_ctl(m_fd, EPOLL_CTL_ADD, event.identifier, &ev) == -1) {
            ERROR_LOG("Failed to add timer event with epoll_ctl: " << strerror(errno));
            return false;
        }
        ++m_events;

        return true;
    }

    bool EventPoll::update(const EventPollTimerElement &event) noexcept {
        // no-op : this is handled by TimerEvent on Linux
        return true;
    }

    bool EventPoll::remove(const EventPollTimerElement &event) noexcept {
        struct epoll_event ev;
        memset(&ev, 0, sizeof(struct epoll_event));

        ev.events = EPOLLIN;
        ev.data.fd = event.identifier;

        if (epoll_ctl(m_fd, EPOLL_CTL_DEL, event.identifier, &ev) == -1) {
            ERROR_LOG("Failed to remove timer event with epoll_ctl: " << strerror(errno));
            return false;
        }
        --m_events;

        return true;
    }

    bool EventPoll::add(const EventPollIOElement &event) noexcept {
        struct epoll_event ev;
        memset(&ev, 0, sizeof(struct epoll_event));

        uint32_t filter = EPOLLET; // we will use edge triggering
        if ((event.filter & EventType::READ) == EventType::READ) {
            filter |= EPOLLIN;
        }
        if ((event.filter & EventType::WRITE) == EventType::WRITE) {
            filter |= EPOLLOUT;
        }

        ev.events = filter;
        ev.data.fd = event.identifier;

        if (epoll_ctl(m_fd, EPOLL_CTL_ADD, event.identifier, &ev) == -1) {
            ERROR_LOG("Failed to add io event with epoll_ctl: " << strerror(errno));
            return false;
        }
        ++m_events;

        return true;
    }

    bool EventPoll::remove(const EventPollIOElement &event) noexcept {
        struct epoll_event ev;
        memset(&ev, 0, sizeof(struct epoll_event));

        int filter = 0;
        if ((event.filter & EventType::READ) == EventType::READ) {
            filter |= EPOLLIN;
        }
        if ((event.filter & EventType::WRITE) == EventType::WRITE) {
            filter |= EPOLLOUT;
        }

        ev.events = filter;
        ev.data.fd = event.identifier;

        if (epoll_ctl(m_fd, EPOLL_CTL_DEL, event.identifier, &ev) == -1) {
            if (errno != EBADF) { // probably the fd is already closed (& epoll with automatically remove it)
                ERROR_LOG("Failed to remove io event with epoll_ctl: " << strerror(errno));
                return false;
            }
        }
        --m_events;

        return true;
    }

    bool EventPoll::add(const EventPollSignalElement &event) noexcept {
        struct epoll_event ev;
        memset(&ev, 0, sizeof(struct epoll_event));

        ev.events = EPOLLIN;
        ev.data.u64 = event.identifier | SignalIdentifier;

        if (epoll_ctl(m_fd, EPOLL_CTL_ADD, event.identifier, &ev) == -1) {
            ERROR_LOG("Failed to add signal event with epoll_ctl: " << strerror(errno));
            return false;
        }
        ++m_events;

        return true;
    }

    bool EventPoll::remove(const EventPollSignalElement &event) noexcept {
        struct epoll_event ev;
        memset(&ev, 0, sizeof(struct epoll_event));

        ev.events = EPOLLIN;
        ev.data.fd = event.identifier;

        if (epoll_ctl(m_fd, EPOLL_CTL_DEL, event.identifier, &ev) == -1) {
            ERROR_LOG("Failed to remove signal event with epoll_ctl: " << strerror(errno));
            return false;
        }
        --m_events;

        return true;
    }

    int EventPoll::run(std::function<void(EventPollIOElement &&)> io_events, std::function<void(EventPollTimerElement &&)> timer_events, std::function<void(EventPollSignalElement &&)> signal_events) noexcept {
        int result = -1;

        if (m_events != 0) {
            struct epoll_event _events[maxDispatchSize];
            result = epoll_wait(m_fd, _events, maxDispatchSize, -1);
            if (result > 0) {
                m_err_count = 0;
                for (int j = 0; j < result; ++j) {
                    int filter = EventType::NONE;
                    if ((_events[j].events & EPOLLIN) == EPOLLIN) {
                        filter |= EventType::READ;
                    }
                    if ((_events[j].events & EPOLLOUT) == EPOLLOUT) {
                        filter |= EventType::WRITE;
                    }

                    if ((_events[j].data.u64 & TimerIdentifier) == TimerIdentifier) {
                        timer_events(EventPollTimerElement(_events[j].data.fd));
                    } else if ((_events[j].data.u64 & SignalIdentifier) == SignalIdentifier) {
                        struct signalfd_siginfo v;
                        ::read(_events[j].data.fd, &v, sizeof(struct signalfd_siginfo));

                        signal_events(EventPollSignalElement(_events[j].data.fd, v.ssi_signo));
                    } else if (filter != EventType::NONE) {
                        io_events(EventPollIOElement(_events[j].data.fd, filter));
                    }
                }
            } else if (result == -1) {
                if (errno == EINTR && m_err_count < 10) {
                    ++m_err_count;
                    return 0;
                }
                ERROR_LOG("epoll_wait returned -1: " << strerror(errno));
                return -1;
            }
        }
        return 0;
    }
}

