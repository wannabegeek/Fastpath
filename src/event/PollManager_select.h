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

#ifndef TFFIXEngine_TFEventPollManager_select_h
#define TFFIXEngine_TFEventPollManager_select_h

#include <chrono>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <thread>
#include <unistd.h>
#include <math.h>
#include <algorithm>
#include <array>

#include <iostream>
#include "EventType.h"

#ifdef __APPLE__

#   include <sys/event.h>

#else
//#   include <sys/epoll.h>
#endif

#ifndef FD_COPY
#   include <cstring>
#   define FD_COPY(src, dest) memcpy((dest), (src), sizeof *(dest))
#endif

namespace DCF {

    template<int MAX_EVENTS>
    class EventPoll {
    private:
        fd_set m_readSet;
        fd_set m_writeSet;
        int m_fdmax = 0;
    public:
        static constexpr bool can_add_events_async = false;
        static constexpr bool can_remove_events_async = false;

        EventPoll() {
            FD_ZERO(&m_readSet);
            FD_ZERO(&m_writeSet);
        }

        ~EventPoll() {
        }

        bool add(const EventPollElement &event) {
            if ((event.filter & EventType::READ) == EventType::READ) {
                FD_SET(event.fd, &m_readSet);
            }
            if ((event.filter & EventType::WRITE) == EventType::WRITE) {
                FD_SET(event.fd, &m_writeSet);
            }

            m_fdmax = std::max(m_fdmax, event.fd);
            return true;
        }

        bool remove(const EventPollElement &event) {
            if ((event.filter & EventType::READ) == EventType::READ) {
                FD_CLR(event.fd, &m_readSet);
            }
            if ((event.filter & EventType::WRITE) == EventType::WRITE) {
                FD_CLR(event.fd, &m_writeSet);
            }

            if (m_fdmax == event.fd) {
                // we need to re-calculate our max fd
                for (int i = 0; i < event.fd; i++) {
                    if (FD_ISSET(i, &m_readSet) || FD_ISSET(i, &m_writeSet)) {
                        m_fdmax = std::max(i, m_fdmax);
                    }
                }
            }
            return true;
        }

        int run(std::array<EventPollElement, MAX_EVENTS> *events, int &numEvents,
                const std::chrono::steady_clock::duration &duration) {
            int result = 0;
            struct timeval *timeoutPtr = nullptr;
            struct timeval timeout = {0, 0};

            if (duration != DistantFuture) {
                if (duration.count() == 0) {
                    timeout = {0, 0};
                } else {
                    timeout.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
                    timeout.tv_usec = static_cast<decltype(timeout.tv_usec)>(
                            std::chrono::duration_cast<std::chrono::microseconds>(duration).count() -
                            (timeout.tv_sec * 1000000));
                }
                timeoutPtr = &timeout;
            }

            fd_set tmpReadSet;
            fd_set tmpWriteSet;

            FD_COPY(&m_readSet, &tmpReadSet);
            FD_COPY(&m_writeSet, &tmpWriteSet);
            while ((result = ::select(m_fdmax + 1, &tmpReadSet, &tmpWriteSet, NULL, timeoutPtr)) == -1 &&
                   (errno == EINTR || errno == EAGAIN)) {
                // Select loop error - restarting
            }

            if (result == -1) {
                return -1;
            } else if (result != 0) {
                numEvents = 0;
                for (int j = 0; j <= m_fdmax; ++j) {
                    int filter = EventType::NONE;
                    if (FD_ISSET(j, &tmpReadSet)) {
                        filter |= EventType::READ;
                    }
                    if (FD_ISSET(j, &tmpWriteSet)) {
                        filter |= EventType::WRITE;
                    }

                    if (filter != EventType::NONE) {
                        (*events)[numEvents++] = EventPollElement(j, filter);
                    }
                }
            }

            return 0;
        }

        friend std::ostream &operator<<(std::ostream &out, const EventPoll &event) {
            out << "Event loop registrations:\n";
            for (int i = 0; i <= event.m_fdmax; i++) {
                out << "\t" << i << (FD_ISSET(i, &event.m_readSet) ? "READ " : "") <<
                (FD_ISSET(i, &event.m_writeSet) ? "WRITE" : "") << "\n";
            }
            return out;
        }
    };
}

#endif
