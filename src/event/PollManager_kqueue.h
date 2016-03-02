//
//  TFEventPollManager_kqueue.h
//  TFFIXEngine
//
//  Created by Tom Fewster on 12/07/2013.
//  Copyright (c) 2013 Tom Fewster. All rights reserved.
//

#ifndef TFFIXEngine_TFEventPollManager_kqueue_h
#define TFFIXEngine_TFEventPollManager_kqueue_h

#include "EventType.h"

#include "utils/logger.h"

#include <sys/event.h>
#include <iostream>
#include <cstring>
#include <array>

namespace DCF {
	class EventPoll {
		int m_kq = 0;
		int m_events = 0;
	public:
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

		bool add(const EventPollElement &event) {
			int filter = 0;
            struct kevent ke;

            if ((event.filter & EventType::READ) == EventType::READ) {
                filter |= EVFILT_READ;
            }
            if ((event.filter & EventType::WRITE) == EventType::WRITE) {
                filter |= EVFILT_WRITE;
            }
            ++m_events;

			EV_SET(&ke, event.fd, filter, EV_ADD, 0, 0, NULL);

            int i = kevent(m_kq, &ke, 1, NULL, 0, NULL);
            if (i == -1) {
                ERROR_LOG("Failed to add event to kevent: " << strerror(errno));
                return false;
            }
			return true;
		}

		bool remove(const EventPollElement &event) {
			int filter = 0;
            struct kevent ke;

            if ((event.filter & EventType::READ) == EventType::READ) {
				filter |= EVFILT_READ;
			}
			if ((event.filter & EventType::WRITE) == EventType::WRITE) {
				filter |= EVFILT_WRITE;
			}

			--m_events;
            EV_SET(&ke, event.fd, filter, EV_DELETE, 0, 0, NULL);

            int i = kevent(m_kq, &ke, 1, NULL, 0, NULL);
            if (i == -1) {
                // EBADF is ok here - probably just means the FD has been closed
                // Once a fd is closed the kernel automatically removeds it from the kevent queue
                if (errno != EBADF) {
                    ERROR_LOG("Failed to remove event to kevent: " << strerror(errno));
                    return false;
                }
            }
			return true;
		}

		int run(std::array<EventPollElement, 256> &events, int &numEvents, const std::chrono::steady_clock::duration &duration) {
			int result = -1;

            if (m_events != 0) {
                struct timespec *timeoutPtr = nullptr;
                struct timespec timeout = {0, 0};

                if (duration != DistantFuture) {
                    if (duration.count() == 0) {
                        timeout = {0, 0};
                    } else {
                        timeout.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
                        timeout.tv_nsec = static_cast<decltype(timeout.tv_nsec)>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() - (timeout.tv_sec * 1000000000));
                        TRACE_LOG("Setting timeout: " << timeout.tv_sec << "s " << timeout.tv_nsec << "ns");
                    }
                    timeoutPtr = &timeout;
                }

                struct kevent _events[MAX_EVENTS];
                result = kevent(m_kq, NULL, 0, _events, m_events, timeoutPtr);
                if (result == -1) {
                    ERROR_LOG("kevent returned -1: " << strerror(errno));
                    return -1;
                } else {
                    numEvents = 0;
                    for (int i = 0; i < result; ++i) {
                        int filter = 0;
                        if ((_events[i].filter & EVFILT_READ) == EVFILT_READ) {
                            filter |= EventType::READ;
                        }
                        if ((_events[i].filter & EVFILT_WRITE) == EVFILT_WRITE) {
                            filter |= EventType::WRITE;
                        }
                        if (filter != EventType::NONE) {
                            events[numEvents++] = EventPollElement(static_cast<int>(_events[i].ident), filter);
                        }
                    }
                }
            } else {
                std::this_thread::sleep_for(duration);
            }
			return 0;
		}

		friend std::ostream &operator<<(std::ostream &out, const EventPoll &event) {
			out << "Event loop registrations:\n";
			return out;
		}
	};
}

#endif
