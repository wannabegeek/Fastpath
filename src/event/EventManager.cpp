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
    EventManager::EventManager() : m_ioCallback(std::bind(&EventManager::serviceIOEvent, this, std::placeholders::_1)), m_timerCallback(std::bind(&EventManager::serviceTimerEvent, this, std::placeholders::_1)) {}

	bool isFileDescriptorValid(int fd) {
		return (!(::fcntl(fd, F_GETFL) == -1 && errno != EBADF));
	}

    void EventManager::waitForEvent() {
		int result = 0;
        m_in_event_wait.store(true); // not sure if this should be before the setTimeout() or below - lets be safe
        result = m_eventLoop.run(m_ioCallback, m_timerCallback);
        m_in_event_wait.store(false);
        if (tf::unlikely(result == -1)) {
            ERROR_LOG("We have an error: [errno: " << strerror(errno) << "(" << errno << ")]");
        }
	}

	void EventManager::serviceIOEvent(const EventPollIOElement &event) {
        foreach_event_matching(event, [&](IOEvent *handler) {
            if (!handler->__awaitingDispatch()) {
                handler->__notify(static_cast<EventType>(handler->eventTypes() & event.filter));
            }
        });
	}

    void EventManager::serviceTimerEvent(const EventPollTimerElement &event) {
        foreach_timer_matching(event, [&](TimerEvent *handler) {
            handler->__notify(EventType::NONE);
        });
    }
}
