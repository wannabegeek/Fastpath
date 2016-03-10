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
        std::array<EventPollElement, maxEvents> *m_events;

        bool setTimeout(std::chrono::microseconds &timeout) const;

    protected:
        EventPoll<maxEvents> m_eventLoop;

		virtual void serviceEvent(const EventPollElement &event);
		virtual void serviceTimers();

        virtual void processPendingRegistrations() = 0;
        virtual void foreach_timer(std::function<void(TimerEvent *)> callback) const = 0;
        virtual void foreach_event_matching(const EventPollElement &event, std::function<void(IOEvent *)> callback) const = 0;

        virtual const bool haveHandlers() const = 0;

    public:
		EventManager();
        EventManager(const EventManager &) = delete;
        EventManager &operator=(const EventManager &) = delete;

        virtual ~EventManager();

		virtual void registerHandler(TimerEvent &eventRegistration) = 0;
		virtual void registerHandler(IOEvent &eventRegistration) = 0;
		virtual void unregisterHandler(TimerEvent &handler) = 0;
		virtual void unregisterHandler(IOEvent &handler) = 0;

		virtual void notify(bool wait = false) = 0;

		void waitForEvent();
		template<typename T> void waitForEvent(const T &timeout) {
            waitForEvent(std::chrono::duration_cast<std::chrono::microseconds>(timeout));
		}
	};

    template<> void EventManager::waitForEvent<std::chrono::microseconds>(const std::chrono::microseconds &timeout);
}
#endif /* defined(__TFFIXEngine__TFSessionEventManager__) */
