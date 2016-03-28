//
//  TFEventPollManager_kqueue.h
//  TFFIXEngine
//
//  Created by Tom Fewster on 12/07/2013.
//  Copyright (c) 2013 Tom Fewster. All rights reserved.
//

#ifndef TFFIXEngine_TFEventPollManager_kqueue_h
#define TFFIXEngine_TFEventPollManager_kqueue_h

#include "event/EventType.h"

#include "utils/logger.h"

#include <sys/event.h>
#include <iostream>
#include <cstring>
#include <array>

namespace DCF {
    template<int MAX_EVENTS>
    class EventPoll {
        int m_kq = 0;
        int m_events = 0;

        static constexpr const bool greater_than(const size_t x, const size_t y) { return x >= y; }

        static constexpr size_t maxDispatchSize = 256;

        static_assert(greater_than(MAX_EVENTS, maxDispatchSize), "MAX_EVENTS template parameter not large enough");
        // we will dispatch at most 'maxDispatchSize'
        struct kevent _events[maxDispatchSize];

    public:
        static constexpr bool can_add_events_async = true;
        static constexpr bool can_remove_events_async = true;

        EventPoll() {
            m_kq = kqueue();
            if (m_kq == -1) {
                // kqueue failed
                ERROR_LOG("Failed to create kqueue: " << strerror(errno));
                return;
            }
        }

        ~EventPoll() {
        }

        bool add(const EventPollTimerElement &event) {
            int filter = EVFILT_TIMER;
            struct kevent ke;

            ++m_events;

            EV_SET(&ke, event.identifier, filter, EV_ADD, NOTE_USECONDS, event.timeout.count(), NULL);

            int i = kevent(m_kq, &ke, 1, NULL, 0, NULL);
            if (i == -1) {
                ERROR_LOG("Failed to add event to kevent: " << strerror(errno));
                return false;
            }
            return true;
        }

        bool update(const EventPollTimerElement &event) {
            int filter = EVFILT_TIMER;
            struct kevent ke;

            EV_SET(&ke, event.identifier, filter, EV_ADD, NOTE_USECONDS, event.timeout.count(), NULL);

            int i = kevent(m_kq, &ke, 1, NULL, 0, NULL);
            if (i == -1) {
                ERROR_LOG("Failed to add event to kevent: " << strerror(errno));
                return false;
            }
            return true;
        }

        bool remove(const EventPollTimerElement &event) {
            int filter = EVFILT_TIMER;
            struct kevent ke;

            --m_events;

            EV_SET(&ke, event.identifier, filter, EV_DELETE, 0, 0, NULL);

            int i = kevent(m_kq, &ke, 1, NULL, 0, NULL);
            if (i == -1) {
                ERROR_LOG("Failed to add event to kevent: " << strerror(errno));
                return false;
            }
            return true;
        }

        bool add(const EventPollIOElement &event) {
            int filter = 0;
            struct kevent ke;

            if ((event.filter & EventType::READ) == EventType::READ) {
                filter |= EVFILT_READ;
            }
            if ((event.filter & EventType::WRITE) == EventType::WRITE) {
                filter |= EVFILT_WRITE;
            }
            ++m_events;

            EV_SET(&ke, event.identifier, filter, EV_ADD | EV_CLEAR, 0, 0, NULL);

            int i = kevent(m_kq, &ke, 1, NULL, 0, NULL);
            if (i == -1) {
                ERROR_LOG("Failed to add event to kevent: " << strerror(errno));
                return false;
            }
            return true;
        }

        bool remove(const EventPollIOElement &event) {
            int filter = 0;
            struct kevent ke;

            if ((event.filter & EventType::READ) == EventType::READ) {
                filter |= EVFILT_READ;
            }
            if ((event.filter & EventType::WRITE) == EventType::WRITE) {
                filter |= EVFILT_WRITE;
            }

            --m_events;
            EV_SET(&ke, event.identifier, filter, EV_DELETE, 0, 0, NULL);

            int i = kevent(m_kq, &ke, 1, NULL, 0, NULL);
            if (i == -1) {
                // EBADF is ok here - probably just means the FD has been closed
                // Once a fd is closed the kernel automatically removes it from the kevent queue
                if (errno != EBADF) {
                    ERROR_LOG("Failed to remove event to kevent: " << strerror(errno));
                    return false;
                }
            }
            return true;
        }

        int run(std::function<void(EventPollIOElement &&)> io_events, std::function<void(EventPollTimerElement &&)> timer_events) {

            if (m_events != 0) {
                int result = kevent(m_kq, NULL, 0, _events, maxDispatchSize, nullptr);
                if (result == -1) {
                    ERROR_LOG("kevent returned -1: " << strerror(errno));
                    return -1;
                } else {
                    for (int i = 0; i < result; ++i) {
                        int filter = 0;
                        if ((_events[i].filter & EVFILT_READ) == EVFILT_READ) {
                            filter |= EventType::READ;
                        }
                        if ((_events[i].filter & EVFILT_WRITE) == EVFILT_WRITE) {
                            filter |= EventType::WRITE;
                        }
                        if (filter != EventType::NONE) {
                            io_events(std::move(EventPollIOElement(static_cast<int>(_events[i].ident), filter)));
                        } else if ((_events[i].filter & EVFILT_TIMER) == EVFILT_TIMER) {
                            for (int64_t i = 0; i < _events[i].data; i++) {
                                timer_events(std::move(EventPollTimerElement(static_cast<int>(_events[i].ident))));
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
