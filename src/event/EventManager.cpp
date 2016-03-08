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

	EventManager::EventManager() : m_servicingEvents(false), m_servicingTimers(false) {
        m_events = new std::array<EventPollElement, maxEvents>;
	}

	EventManager::~EventManager() {
        delete m_events;
	}

    EventManager::EventManager(EventManager &&other) : m_events(other.m_events), m_eventLoop(other.m_eventLoop), m_servicingEvents(other.m_servicingEvents), m_servicingTimers(other.m_servicingTimers) {
        other.m_events = new std::array<EventPollElement, maxEvents>;
    }

    void EventManager::waitForEvent() {
		this->waitForEvent(DistantFuture);
	}

	template<> void EventManager::waitForEvent<std::chrono::microseconds>(const std::chrono::microseconds &timeout) {
        this->processPendingRegistrations();

		int result = 0;
		if (__builtin_expect(timeout.count() == 0 && !haveHandlers(), false)) {
			ERROR_LOG("No events to handle - returning");
		} else {
			std::chrono::microseconds duration = timeout;
			if (setTimeout(duration)) {
				duration = std::min(duration, timeout);
			}

			int numEvents = 0;
			result = m_eventLoop.run(m_events, numEvents, duration);
			if (result != -1) {
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

        __foreach_timer([&](auto handler) {
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
        m_servicingEvents = true;
        __foreach_ioevent([&](auto handler) {
                if (handler->fileDescriptor() == event.fd) {
                const int events = handler->eventTypes() & event.filter;
                if (events != EventType::NONE && !handler->__awaitingDispatch()) {
					handler->__setAwaitingDispatch(true);
                    handler->__notify(static_cast<EventType>(events));
                }
            }
        });
        m_servicingEvents = false;
	}

	void EventManager::serviceTimers() {
		std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();

		m_servicingTimers = true;
		__foreach_timer([&](auto handler) {
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
		m_servicingTimers = false;
	}

//    bool EventManager::isRegistered(const TimerEvent &handler) const {
//        return std::find(m_timerHandlers.begin(), m_timerHandlers.end(), &handler) != m_timerHandlers.end();
//    }
//
//    bool EventManager::isRegistered(const IOEvent &handler) const {
//        return std::find(m_ioHandlers.begin(), m_ioHandlers.end(), &handler) != m_ioHandlers.end();
//    }
}
