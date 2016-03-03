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

	EventManager::EventManager() : m_servicingEvents(false), m_servicingTimers(false), m_pendingFileDescriptorRegistrationEvents(false), m_pendingTimerRegistrationEvents(false) {
#ifdef HAVE_SYS_EVENTFD_H
		int registerHandlerFd = -1;
		if ((registerHandlerFd = ::eventfd(0, EFD_NONBLOCK)) != -1) {
			m_readFileDescriptor = registerHandlerFd;
			m_writeFileDescriptor = registerHandlerFd;
			startThread();
		} else {
			ERROR_LOG("eventfd creation failed");
		}
	#else
		//	int registerHandlerFd[2];
		//	if (::pipe(registerHandlerFd) != -1) {
		//		m_readFileDescriptor = registerHandlerFd[0];
		//		m_writeFileDescriptor = registerHandlerFd[1];
		//		// start our processing loop
		//		if (fcntl(m_readFileDescriptor, F_SETFL, O_NONBLOCK) != -1 && fcntl(m_writeFileDescriptor, F_SETFL, O_NONBLOCK) != -1) {
		//			startThread();
		//		} else {
		//			ERROR_LOG("Failed to set pipe to non-blocking");
		//		}
		//	} else {
		//		ERROR_LOG("Pipe creation failed");
		//	}
    #endif
		m_eventLoop.add(m_actionNotifier.pollElement());
	}

	EventManager::~EventManager() {
		m_eventLoop.remove(m_actionNotifier.pollElement());
	}

    EventManager::EventManager(EventManager &&other) : m_timerHandlers(other.m_timerHandlers), m_handlers(other.m_handlers), m_eventLoop(other.m_eventLoop), m_events(other.m_events), m_servicingEvents(other.m_servicingEvents), m_servicingTimers(other.m_servicingTimers), m_pendingFileDescriptorRegistrationEvents(false), m_pendingTimerRegistrationEvents(false) {
        other.m_timerHandlers.clear();
        other.m_handlers.clear();
        other.m_events.fill(EventPollElement());
    }

    void EventManager::waitForEvent() {
		this->waitForEvent(DistantFuture);
	}

	template<> void EventManager::waitForEvent<std::chrono::microseconds>(const std::chrono::microseconds &timeout) {
		if (m_pendingTimerRegistrationEvents) {
			m_timerHandlers = m_pendingTimerHandlers;
			m_pendingTimerRegistrationEvents = false;
		}
		if (m_pendingFileDescriptorRegistrationEvents) {
			m_handlers = m_pendingHandlers;
			m_pendingFileDescriptorRegistrationEvents = false;
		}

		int result = 0;
		if (__builtin_expect(timeout.count() == 0 && m_handlers.size() == 0 && m_timerHandlers.size() == 0, false)) {
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
					serviceEvent(m_events[i]);
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

		std::for_each(m_timerHandlers.begin(), m_timerHandlers.end(), [&](const TimerEvent *handler) {
//			TimerEvent *handler = const_cast<TimerEvent *>(h);
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
        if (event.fd == m_actionNotifier.read_handle()) {
            m_actionNotifier.reset();
        } else {
            m_servicingEvents = true;
            for (IOEvent *handler : m_handlers) {
                if (handler->fileDescriptor() == event.fd) {
                    const int events = handler->eventTypes() & event.filter;
                    if (events != EventType::NONE) {
                        handler->__notify(static_cast<EventType>(events));
                    }
                }
            }
            m_servicingEvents = false;
        }
	}

	void EventManager::serviceTimers() {
		std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();

		m_servicingTimers = true;
		for(TimerEvent *handler : m_timerHandlers) {
			if (handler->m_timeoutState == TimerEvent::TIMEOUTSTATE_PROGRESS) {
				// calculate the elapsed time
				std::chrono::microseconds elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - handler->m_lastTime);

				handler->m_timeLeft = handler->m_timeLeft - elapsedTime;

				if (handler->m_timeLeft <= std::chrono::microseconds(0)) {
                    handler->m_timeLeft = handler->m_timeout;
					handler->__notify(EventType::NONE);    //Execute the handler
				}
			}
		}
		m_servicingTimers = false;
	}

	void EventManager::registerHandler(TimerEvent &eventRegistration) {
		if (m_servicingTimers) {
			m_pendingTimerRegistrationEvents = true;
			m_pendingTimerHandlers.push_back(&eventRegistration);
		} else {
			m_timerHandlers.push_back(&eventRegistration);
			m_pendingTimerHandlers.push_back(&eventRegistration);
		}
	}

	void EventManager::registerHandler(IOEvent &eventRegistration) {
        if (eventRegistration.fileDescriptor() <= 0) {
            ERROR_LOG("Failed to register invalid file descriptor: " << eventRegistration.fileDescriptor());
            throw EventException("Invalid file descriptor");
        }

		if (m_servicingEvents) {
			m_pendingHandlers.push_back(&eventRegistration);
			m_pendingFileDescriptorRegistrationEvents = true;
		} else {
			m_handlers.push_back(&eventRegistration);
			m_pendingHandlers.push_back(&eventRegistration);
		}
		m_eventLoop.add({eventRegistration.fileDescriptor(), eventRegistration.eventTypes()});
	}

	void EventManager::unregisterHandler(const TimerEvent &handler) {
		auto it = std::find(m_pendingTimerHandlers.begin(), m_pendingTimerHandlers.end(), &handler);
		if (it != m_pendingTimerHandlers.end()) {
			m_pendingTimerHandlers.erase(it);
		}
		if (m_servicingTimers) {
			m_pendingTimerRegistrationEvents = true;
		} else {
			auto it = std::find(m_timerHandlers.begin(), m_timerHandlers.end(), &handler);
			if (it != m_timerHandlers.end()) {
				m_timerHandlers.erase(it);
			}
		}
	}

	void EventManager::unregisterHandler(const IOEvent &handler) {
		auto it = std::find(m_pendingHandlers.begin(), m_pendingHandlers.end(), &handler);
        if (it != m_pendingHandlers.end()) {
			m_pendingHandlers.erase(it);

            if (handler.eventTypes() != EventType::ALL) {
                it = std::find_if(m_pendingHandlers.begin(), m_pendingHandlers.end(), [&handler](const IOEvent *other) {
                    return other->fileDescriptor() == handler.fileDescriptor() && (other->eventTypes() & handler.eventTypes()) == handler.eventTypes();
                });
                if (it == m_pendingHandlers.end()) {
                    m_eventLoop.remove({handler.fileDescriptor(), handler.eventTypes()});
                }
            } else {
                EventType eventTypes = handler.eventTypes();
                std::for_each(m_pendingHandlers.begin(), m_pendingHandlers.end(), [&eventTypes, &handler](const IOEvent *other) {
                    if (other->fileDescriptor() == handler.fileDescriptor()) {
                        eventTypes = static_cast<EventType>(eventTypes & ~other->eventTypes());
                    }
                });
                if (eventTypes != EventType::NONE) {
                    m_eventLoop.remove({handler.fileDescriptor(), eventTypes});
                }
            }
        }

		if (m_servicingEvents) {
			m_pendingFileDescriptorRegistrationEvents = true;
		} else {
			auto it = std::find(m_handlers.begin(), m_handlers.end(), &handler);
			if (it != m_handlers.end()) {
				m_handlers.erase(it);
			}
		}
	}

    bool EventManager::isRegistered(const TimerEvent &handler) const {
        return std::find(m_timerHandlers.begin(), m_timerHandlers.end(), &handler) != m_timerHandlers.end();
    }

    bool EventManager::isRegistered(const IOEvent &handler) const {
        return std::find(m_handlers.begin(), m_handlers.end(), &handler) != m_handlers.end();
    }

	void EventManager::notify() {
		m_actionNotifier.notify();
	}
}
