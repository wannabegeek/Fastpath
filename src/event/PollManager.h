//
//  TFEventPollManager.h
//  TFFIXEngine
//
//  Created by Tom Fewster on 12/07/2013.
//  Copyright (c) 2013 Tom Fewster. All rights reserved.
//

#ifndef __TFFIXEngine__TFEventPollManager__
#define __TFFIXEngine__TFEventPollManager__

#include <chrono>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <thread>
#include <unistd.h>
#include <math.h>
#include <algorithm>

#include "../config.h"

namespace DCF {
	using DistantFutureType = std::chrono::microseconds;
	static constexpr DistantFutureType DistantFuture = DistantFutureType::max();

	struct EventPollIOElement {
		EventPollIOElement() {}
		EventPollIOElement(const int _fd, const int _filter) : identifier(_fd), filter(_filter) {}

		int identifier = -1;
		int filter = 0;
	};

	struct EventPollTimerElement {
        EventPollTimerElement() {}
        EventPollTimerElement(const int _fd, const std::chrono::microseconds _timeout) : identifier(_fd), timeout(_timeout) {}
        EventPollTimerElement(const int _fd) : identifier(_fd) {}

		int identifier = -1;
        std::chrono::microseconds timeout;
	};
}

#if defined HAVE_KEVENT
#	include "event/arch/mac/PollManager.h"
#elif defined HAVE_EPOLL
#	include "event/arch/linux/PollManager.h"
#else
#error "Requires either kqueue or epoll"
//#	include "PollManager_select.h"
#endif

#endif /* defined(__TFFIXEngine__TFEventPollManager__) */
