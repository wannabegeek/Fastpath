//
//  TFSessionEventManager.cpp
//  TFFIXEngine
//
//  Created by Tom Fewster on 13/04/2013.
//  Copyright (c) 2013 Tom Fewster. All rights reserved.
//

#include "EventManager.h"
#include "utils/logger.h"

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "PollManager.h"

#include <thread>
#include <unistd.h>
#include <iostream>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <array>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <utils/optimize.h>


#ifdef HAVE_SYS_EVENTFD_H
#	include <sys/eventfd.h>
#endif

#include "EventType.h"
#include "TimerEvent.h"
#include "IOEvent.h"

namespace DCF {
	bool isFileDescriptorValid(int fd) {
		return (!(::fcntl(fd, F_GETFL) == -1 && errno != EBADF));
	}

	EventManager::EventManager() {
        m_events = new std::array<EventPollElement, maxEvents>;
	}

	EventManager::~EventManager() {
        delete m_events;
	}

    void EventManager::waitForEvent() {
		this->waitForEvent(DistantFuture);
	}

	template<> void EventManager::waitForEvent<std::chrono::microseconds>(const std::chrono::microseconds &timeout) {
        this->processPendingRegistrations();

		int result = 0;
		if (tf::unlikely(timeout.count() == 0 && !haveHandlers())) {
			ERROR_LOG("No events to handle - returning");
		} else {
            m_in_event_wait.store(true); // not sure if this should be before the setTimeout() or below - lets be safe
			std::chrono::microseconds duration = timeout;
			if (setTimeout(duration)) {
				duration = std::min(duration, timeout);
			}

			int numEvents = 0;
			result = m_eventLoop.run(m_events, numEvents, duration);
            m_in_event_wait.store(false);
			if (tf::likely(result != -1)) {
				for (int i = 0; i < numEvents; ++i) {
					serviceEvent(m_events->at(i));
				}
				serviceTimers();
			} else {
				ERROR_LOG("We have an error: [errno: " << strerror(errno) << "(" << errno << ")]");
			}
		}
	}

	bool EventManager::setTimeout(std::chrono::microseconds &timeout) const {
		std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();
		std::chrono::microseconds duration = timeout;
		bool haveTimeout = false;

        foreach_timer([&](auto handler) {
            if (handler->m_timeoutState == TimerEvent::TIMEOUTSTATE_START) {
                haveTimeout = true;
                handler->m_lastTime = nowTime;
                handler->m_timeoutState = TimerEvent::TIMEOUTSTATE_PROGRESS;
                if (handler->m_timeLeft < duration) {
                    duration = handler->m_timeLeft;
                }
            } else if (handler->m_timeoutState == TimerEvent::TIMEOUTSTATE_PROGRESS) {
                haveTimeout = true;
                handler->m_lastTime = nowTime;
                if (handler->m_timeLeft < duration) {
                    duration = handler->m_timeLeft;
                }
            }
        });

		if (haveTimeout) {
			timeout = duration;
		}
		return haveTimeout;
	}

	void EventManager::serviceEvent(const EventPollElement &event) {
        foreach_event_matching(event, [&](IOEvent *handler) {
            if (!handler->__awaitingDispatch()) {
                handler->__setAwaitingDispatch(true);
                handler->__notify(static_cast<EventType>(handler->eventTypes() & event.filter));
            }
        });
	}

	void EventManager::serviceTimers() {
		std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();

        foreach_timer([&](auto handler) {
            if (handler->m_timeoutState == TimerEvent::TIMEOUTSTATE_PROGRESS) {
                // calculate the elapsed time
                std::chrono::microseconds elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(
                        nowTime - handler->m_lastTime);

                handler->m_timeLeft = handler->m_timeLeft - elapsedTime;

                if (handler->m_timeLeft <= std::chrono::microseconds(0)) {
                    handler->m_timeLeft = handler->m_timeout;
                    if (!handler->__awaitingDispatch()) {
                        handler->__setAwaitingDispatch(true);
                        handler->__notify(EventType::NONE);    //Execute the handler
                    }
                }
            }
        });
	}
}
