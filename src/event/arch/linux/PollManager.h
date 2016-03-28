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
        int m_epolltimerfd = -1;
        int m_events = 0;
        int m_timer_events = 0;
        int err_count = 0;

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

            m_epolltimerfd = epoll_create1(0);
            if (m_epolltimerfd == -1) {
                ERROR_LOG("Failed to create epoll fd: " << strerror(errno));
                return;
            }

            struct epoll_event ev;
            memset(&ev, 0, sizeof(struct epoll_event));
            ev.events = EPOLLIN;
            ev.data.fd = m_epolltimerfd;
            if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, event.fd, &ev) == -1) {
                ERROR_LOG("Failed to add event with epoll_ctl: " << strerror(errno));
                return false;
            }
            ++m_events;
        }

        ~EventPoll() {
        }

        bool add(const EventPollTimerElement &event) {
            struct epoll_event ev;
            memset(&ev, 0, sizeof(struct epoll_event));
            ++m_events;

            ev.events = EPOLLIN;
            ev.data.fd = event.identifier;

            if (epoll_ctl(m_epolltimerfd, EPOLL_CTL_ADD, event.fd, &ev) == -1) {
                ERROR_LOG("Failed to add event with epoll_ctl: " << strerror(errno));
                return false;
            }

            return true;
        }

        bool update(const EventPollTimerElement &event) {
            // no-op : this is handled by TimerEvent on Linux
            return true;
        }

        bool remove(const EventPollTimerElement &event) {
            struct epoll_event ev;
            memset(&ev, 0, sizeof(struct epoll_event));
            --m_events;

            ev.events = EPOLLIN;
            ev.data.fd = event.identifier;

            if (epoll_ctl(m_epolltimerfd, EPOLL_CTL_DEL, event.fd, &ev) == -1) {
                ERROR_LOG("Failed to remove event with epoll_ctl: " << strerror(errno));
                return false;
            }

            return true;
        }

        bool add(const EventPollElement &event) {
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

            if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, event.fd, &ev) == -1) {
                ERROR_LOG("Failed to add event with epoll_ctl: " << strerror(errno));
                return false;
            }

            return true;
        }

        bool remove(const EventPollElement &event) {
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

            if (epoll_ctl(m_epollfd, EPOLL_CTL_DEL, event.fd, &ev) == -1) {
                if (errno != EBADF) { // probably the fd is already closed (& epoll with automatically remove it)
                    ERROR_LOG("Failed to remove event with epoll_ctl: " << strerror(errno));
                    return false;
                }
            }

            return true;
        }

        int run(std::function<void(EventPollIOElement &&)> io_events, std::function<void(EventPollTimerElement &&)> timer_events) {
            int result = -1;

            if (m_events != 0) {
                result = epoll_wait(m_epollfd, _events, m_events, nullptr);
                if (result == -1) {
                    if (errno == EINTR && err_count < 10) {
                        ++err_count;
                        return 0;
                    }
                    ERROR_LOG("epoll_wait returned -1: " << strerror(errno));
                    return -1;
                } else {
                    err_count = 0;
                    numEvents = 0;
                    for (int j = 0; j < result; ++j) {
                        if (_events[i].ident == m_epolltimerfd) {
                            struct epoll_event _timer_events[maxDispatchSize];
                            result2 = epoll_wait(m_epolltimerfd, _timer_events, m_timer_events, nullptr);
                            for (int i = 0; i < result2; ++i) {
                                timer_events(std::move(EventPollTimerElement(_timer_events[j].data.fd)));
                            }

//                            if (filter != EventType::NONE) {
//                            } else if ((_events[i].filter & EVFILT_TIMER) == EVFILT_TIMER) {
//                                for (int64_t i = 0; i < _events[i].data; i++) {
//                                    timer_events(std::move(EventPollTimerElement(static_cast<int>(_events[i].ident))));
//                                }
//                            }
                        } else {
                            int filter = EventType::NONE;
                            if ((_events[j].events & EPOLLIN) == EPOLLIN) {
                                filter |= EventType::READ;
                            }
                            if ((_events[j].events & EPOLLOUT) == EPOLLOUT) {
                                filter |= EventType::WRITE;
                            }

                            if (filter != EventType::NONE) {
                                io_events(std::move(EventPollIOElement(_events[j].data.fd, filter)));
                            }
                        }
                    }
                }
            }
            return 0;
        }
    };
}

#endif
