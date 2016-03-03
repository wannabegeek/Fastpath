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
	private:
		EventManager(const EventManager &) = delete;
		EventManager &operator=(const EventManager &) = delete;

		bool setTimeout(std::chrono::microseconds &timeout) const;
		void serviceEvent(const EventPollElement &event);
		void serviceTimers();

		std::vector<TimerEvent *> m_timerHandlers;
		std::vector<IOEvent *> m_handlers;

		EventPoll m_eventLoop;
		std::array<EventPollElement, 256> m_events;

		mutable bool m_servicingEvents;
		mutable bool m_servicingTimers;

		ActionNotifier m_actionNotifier;

		bool m_pendingFileDescriptorRegistrationEvents;
		bool m_pendingTimerRegistrationEvents;
		std::vector<TimerEvent *> m_pendingTimerHandlers;
		std::vector<IOEvent *> m_pendingHandlers;

	public:
		EventManager();
		~EventManager();
        EventManager(EventManager &&other);

		void registerHandler(TimerEvent &eventRegistration);
		void registerHandler(IOEvent &eventRegistration);
//		void emplaceHandler(const std::chrono::microseconds &timeout, const std::function<void(int)> &callback);
//		void emplaceHandler(const int fd, const EventType eventType, const std::function<void(int)> &callback);
		void unregisterHandler(const TimerEvent &handler);
		void unregisterHandler(const IOEvent &handler);

        bool isRegistered(const TimerEvent &handler) const;
        bool isRegistered(const IOEvent &handler) const;

		void notify();

		void waitForEvent();
		template<typename T> void waitForEvent(const T &timeout) {
            waitForEvent(std::chrono::duration_cast<std::chrono::microseconds>(timeout));
		}
	};

    template<> void EventManager::waitForEvent<std::chrono::microseconds>(const std::chrono::microseconds &timeout);
}
#endif /* defined(__TFFIXEngine__TFSessionEventManager__) */
