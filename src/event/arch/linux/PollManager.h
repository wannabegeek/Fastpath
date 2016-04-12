//
//  TFEventPollManager_epoll.h
//  TFFIXEngine
//
//  Created by Tom Fewster on 12/07/2013.
//  Copyright (c) 2013 Tom Fewster. All rights reserved.
//

#ifndef TFFIXEngine_TFEventPollManager_epoll_h
#define TFFIXEngine_TFEventPollManager_epoll_h

#include "event/EventType.h"
#include "utils/logger.h"

#include <thread>
#include <sys/epoll.h>
#include <iostream>
#include <cstring>

namespace DCF {
    template<int MAX_EVENTS>
    class EventPoll {
    private:
        int m_epollfd = -1;
        int m_events = 0;
        int err_count = 0;

        static constexpr uint64_t TimerIdentifier = 1l << 33;

        static constexpr const bool greater_than(const size_t x, const size_t y) { return x >= y; }

        static constexpr size_t maxDispatchSize = 256;

        static_assert(greater_than(MAX_EVENTS, maxDispatchSize), "MAX_EVENTS template parameter not large enough");
        // we will dispatch at most 'maxDispatchSize'
        struct epoll_event _events[maxDispatchSize];

    public:
        static constexpr bool can_add_events_async = true;
        static constexpr bool can_remove_events_async = true;

        EventPoll() {
            m_epollfd = epoll_create1(0);
            if (m_epollfd == -1) {
                ERROR_LOG("Failed to create epoll fd: " << strerror(errno));
                return;
            }
        }

        ~EventPoll() {
        }

        bool add(const EventPollTimerElement &event) noexcept {
            struct epoll_event ev;
            memset(&ev, 0, sizeof(struct epoll_event));
            ++m_events;

            ev.events = EPOLLIN;
            ev.data.u64 = event.identifier | TimerIdentifier;

            if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, event.identifier, &ev) == -1) {
                ERROR_LOG("Failed to add event with epoll_ctl: " << strerror(errno));
                return false;
            }

            return true;
        }

        bool update(const EventPollTimerElement &event) noexcept {
            // no-op : this is handled by TimerEvent on Linux
            return true;
        }

        bool remove(const EventPollTimerElement &event) noexcept {
            struct epoll_event ev;
            memset(&ev, 0, sizeof(struct epoll_event));
            --m_events;

            ev.events = EPOLLIN;
            ev.data.fd = event.identifier;

            if (epoll_ctl(m_epollfd, EPOLL_CTL_DEL, event.identifier, &ev) == -1) {
                ERROR_LOG("Failed to remove event with epoll_ctl: " << strerror(errno));
                return false;
            }

            return true;
        }

        bool add(const EventPollIOElement &event) noexcept {
            struct epoll_event ev;
            memset(&ev, 0, sizeof(struct epoll_event));

            uint32_t filter = EPOLLET; // we will use edge triggering
            if ((event.filter & EventType::READ) == EventType::READ) {
                filter |= EPOLLIN;
            }
            if ((event.filter & EventType::WRITE) == EventType::WRITE) {
                filter |= EPOLLOUT;
            }
            ++m_events;

            ev.events = filter;
            ev.data.fd = event.identifier;

            if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, event.identifier, &ev) == -1) {
                ERROR_LOG("Failed to add event with epoll_ctl: " << strerror(errno));
                return false;
            }

            return true;
        }

        bool remove(const EventPollIOElement &event) noexcept {
            struct epoll_event ev;
            memset(&ev, 0, sizeof(struct epoll_event));

            int filter = 0;
            if ((event.filter & EventType::READ) == EventType::READ) {
                filter |= EPOLLIN;
            }
            if ((event.filter & EventType::WRITE) == EventType::WRITE) {
                filter |= EPOLLOUT;
            }
            --m_events;

            ev.events = filter;
            ev.data.fd = event.identifier;

            if (epoll_ctl(m_epollfd, EPOLL_CTL_DEL, event.identifier, &ev) == -1) {
                if (errno != EBADF) { // probably the fd is already closed (& epoll with automatically remove it)
                    ERROR_LOG("Failed to remove event with epoll_ctl: " << strerror(errno));
                    return false;
                }
            }

            return true;
        }

        int run(std::function<void(EventPollIOElement &&)> io_events, std::function<void(EventPollTimerElement &&)> timer_events) noexcept {
            int result = -1;

            if (m_events != 0) {
                result = epoll_wait(m_epollfd, _events, m_events, 0);
                if (result == -1) {
                    if (errno == EINTR && err_count < 10) {
                        ++err_count;
                        return 0;
                    }
                    ERROR_LOG("epoll_wait returned -1: " << strerror(errno));
                    return -1;
                } else {
                    err_count = 0;
                    for (int j = 0; j < result; ++j) {
                        int filter = EventType::NONE;
                        if ((_events[j].events & EPOLLIN) == EPOLLIN) {
                            filter |= EventType::READ;
                        }
                        if ((_events[j].events & EPOLLOUT) == EPOLLOUT) {
                            filter |= EventType::WRITE;
                        }

                        if ((_events[j].data.u64 & TimerIdentifier) == TimerIdentifier) {
                            timer_events(EventPollTimerElement(_events[j].data.u64 & ~TimerIdentifier));
                        } else if (filter != EventType::NONE) {
                            io_events(EventPollIOElement(_events[j].data.fd, filter));
                        }
                    }
                }
            }
            return 0;
        }
    };
}

#endif