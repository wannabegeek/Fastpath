//
//  TFSessionEventManager.h
//  TFFIXEngine
//
//  Created by Tom Fewster on 13/04/2013.
//  Copyright (c) 2013 Tom Fewster. All rights reserved.
//

#ifndef __TFFIXEngine__TFSessionEventManager__
#define __TFFIXEngine__TFSessionEventManager__

#include "PollManager.h"
#include "ActionNotifier.h"

#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <array>
#include <unistd.h>
#include <atomic>

#include <type_traits>

namespace DCF {
    class TimerEvent;
    class IOEvent;
    
	class EventManager {
    public:
        static constexpr const size_t maxEvents = std::numeric_limits<uint16_t>::max();

    protected:
        EventPoll<maxEvents> m_eventLoop;

        std::atomic<bool> m_in_event_wait = ATOMIC_VAR_INIT(false);

        virtual void serviceIOEvent(const EventPollIOElement &event);
        virtual void serviceTimerEvent(const EventPollTimerElement &event);

        virtual void foreach_event_matching(const EventPollIOElement &event, std::function<void(IOEvent *)> callback) const = 0;
        virtual void foreach_timer_matching(const EventPollTimerElement &event, std::function<void(TimerEvent *)> callback) const = 0;

        virtual const bool haveHandlers() const = 0;

    public:
        EventManager() {}

        EventManager(const EventManager &) = delete;
        EventManager &operator=(const EventManager &) = delete;

        virtual ~EventManager() {};

        virtual void registerHandler(TimerEvent *eventRegistration) = 0;
		virtual void registerHandler(IOEvent *eventRegistration) = 0;
        virtual void updateHandler(TimerEvent *eventRegistration) = 0;
		virtual void unregisterHandler(TimerEvent *handler) = 0;
		virtual void unregisterHandler(IOEvent *handler) = 0;

		virtual void notify(bool wait = false) = 0;

		void waitForEvent();
	};

}
#endif /* defined(__TFFIXEngine__TFSessionEventManager__) */
