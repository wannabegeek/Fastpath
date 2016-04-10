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

	private:
		std::function<void(EventPollIOElement &&)> m_ioCallback;
		std::function<void(EventPollTimerElement &&)> m_timerCallback;
    protected:
        EventPoll<maxEvents> m_eventLoop;

        std::atomic<bool> m_in_event_wait = ATOMIC_VAR_INIT(false);

        virtual void serviceIOEvent(const EventPollIOElement &event) noexcept;
        virtual void serviceTimerEvent(const EventPollTimerElement &event) noexcept;

        virtual void foreach_event_matching(const EventPollIOElement &event, std::function<void(IOEvent *)> callback) const noexcept = 0;
        virtual void foreach_timer_matching(const EventPollTimerElement &event, std::function<void(TimerEvent *)> callback) const noexcept = 0;

        virtual const bool haveHandlers() const noexcept = 0;

    public:
        EventManager();

        EventManager(const EventManager &) = delete;
        EventManager &operator=(const EventManager &) = delete;

        virtual ~EventManager() {};

        virtual void registerHandler(TimerEvent *eventRegistration) noexcept = 0;
		virtual void registerHandler(IOEvent *eventRegistration) noexcept = 0;
        virtual void updateHandler(TimerEvent *eventRegistration) noexcept = 0;
		virtual void unregisterHandler(TimerEvent *handler) noexcept = 0;
		virtual void unregisterHandler(IOEvent *handler) noexcept = 0;

		virtual void notify(bool wait = false) noexcept = 0;

		void waitForEvent() noexcept;
	};

}
#endif /* defined(__TFFIXEngine__TFSessionEventManager__) */
