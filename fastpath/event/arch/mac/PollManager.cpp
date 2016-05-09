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

#include <iostream>
#include <cstring>
#include <array>
#include <sys/event.h>

namespace fp {
    EventPoll::EventPoll() : m_err_count(0) {
        m_fd = kqueue();
        if (m_fd == -1) {
            // kqueue failed
            m_err_count = 0;
            ERROR_LOG("Failed to create kqueue: " << strerror(errno));
            return;
        }
    }

    bool EventPoll::add(const EventPollTimerElement &event) noexcept {
        struct kevent ke;

        EV_SET(&ke, event.identifier, EVFILT_TIMER, EV_ADD, NOTE_USECONDS, event.timeout.count(), NULL);

        int i = kevent(m_fd, &ke, 1, NULL, 0, NULL);
        if (i == -1) {
            ERROR_LOG("Failed to add event to kevent: " << strerror(errno));
            return false;
        }
        ++m_events;

        return true;
    }

    bool EventPoll::update(const EventPollTimerElement &event) noexcept {
        struct kevent ke;

        EV_SET(&ke, event.identifier, EVFILT_TIMER, EV_ADD, NOTE_USECONDS, event.timeout.count(), NULL);

        int i = kevent(m_fd, &ke, 1, NULL, 0, NULL);
        if (i == -1) {
            ERROR_LOG("Failed to add event to kevent: " << strerror(errno));
            return false;
        }
        return true;
    }

    bool EventPoll::remove(const EventPollTimerElement &event) noexcept {
        struct kevent ke;

        EV_SET(&ke, event.identifier, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);

        int i = kevent(m_fd, &ke, 1, NULL, 0, NULL);
        if (i == -1) {
            ERROR_LOG("Failed to add event to kevent: " << strerror(errno));
            return false;
        }
        --m_events;

        return true;
    }

    bool EventPoll::add(const EventPollIOElement &event) noexcept {
        int filter = 0;
        struct kevent ke;

        if ((event.filter & EventType::READ) == EventType::READ) {
            filter |= EVFILT_READ;
        }
        if ((event.filter & EventType::WRITE) == EventType::WRITE) {
            filter |= EVFILT_WRITE;
        }

        EV_SET(&ke, event.identifier, filter, EV_ADD | EV_CLEAR, 0, 0, NULL);

        int i = kevent(m_fd, &ke, 1, NULL, 0, NULL);
        if (i == -1) {
            ERROR_LOG("Failed to add event to kevent: " << strerror(errno));
            return false;
        }
        ++m_events;

        return true;
    }

    bool EventPoll::remove(const EventPollIOElement &event) noexcept {
        int filter = 0;
        struct kevent ke;

        if ((event.filter & EventType::READ) == EventType::READ) {
            filter |= EVFILT_READ;
        }
        if ((event.filter & EventType::WRITE) == EventType::WRITE) {
            filter |= EVFILT_WRITE;
        }

        EV_SET(&ke, event.identifier, filter, EV_DELETE, 0, 0, NULL);

        int i = kevent(m_fd, &ke, 1, NULL, 0, NULL);
        if (i == -1) {
            // EBADF is ok here - probably just means the FD has been closed
            // Once a fd is closed the kernel automatically removes it from the kevent queue
            if (errno != EBADF) {
                ERROR_LOG("Failed to remove event to kevent: " << strerror(errno));
                return false;
            }
        }
        --m_events;

        return true;
    }

    bool EventPoll::add(const EventPollSignalElement &event) noexcept {
        struct kevent ke;

        EV_SET(&ke, event.signal, EVFILT_SIGNAL, EV_ADD, 0, 0, NULL);

        int i = kevent(m_fd, &ke, 1, NULL, 0, NULL);
        if (i == -1) {
            ERROR_LOG("Failed to add event to kevent: " << strerror(errno));
            return false;
        }
        ++m_events;

        return true;
    }

    bool EventPoll::remove(const EventPollSignalElement &event) noexcept {
        struct kevent ke;

        EV_SET(&ke, event.identifier, EVFILT_SIGNAL, EV_DELETE, 0, 0, NULL);

        int i = kevent(m_fd, &ke, 1, NULL, 0, NULL);
        if (i == -1) {
            ERROR_LOG("Failed to add event to kevent: " << strerror(errno));
            return false;
        }
        --m_events;

        return true;
    }

    int EventPoll::run(const std::function<void(EventPollIOElement &&)> &io_events, const std::function<void(EventPollTimerElement &&)> &timer_events, const std::function<void(EventPollSignalElement &&)> &signal_events) noexcept {

        if (m_events != 0) {
            struct kevent _events[maxDispatchSize];
            int result = kevent(m_fd, NULL, 0, _events, maxDispatchSize, nullptr);
            if (result > 0) {
                m_err_count = 0;
                for (int i = 0; i < result; ++i) {
                    int filter = 0;
                    if ((_events[i].filter & EVFILT_READ) == EVFILT_READ) {
                        filter |= EventType::READ;
                    }
                    if ((_events[i].filter & EVFILT_WRITE) == EVFILT_WRITE) {
                        filter |= EventType::WRITE;
                    }
                    if (filter != EventType::NONE) {
                        io_events(EventPollIOElement(static_cast<int>(_events[i].ident), filter));
                    } else if ((_events[i].filter & EVFILT_TIMER) == EVFILT_TIMER) {
                        for (int64_t i = 0; i < _events[i].data; i++) {
                            timer_events(EventPollTimerElement(static_cast<int>(_events[i].ident)));
                        }
                    } else if ((_events[i].filter & EVFILT_SIGNAL) == EVFILT_SIGNAL) {
                        for (int64_t i = 0; i < _events[i].data; i++) {
                            signal_events(EventPollSignalElement(-1, static_cast<int>(_events[i].ident)));
                        }
                    }
                }
            } else if (result == -1) {
                if (errno == EINTR && m_err_count < 10) {
                    ++m_err_count;
                    return 0;
                }
                ERROR_LOG("kevent returned -1: " << strerror(errno));
                return -1;
            }
        }
        return 0;
    }
}
