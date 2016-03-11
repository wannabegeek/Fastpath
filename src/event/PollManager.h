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

	struct EventPollElement {
		EventPollElement() {}
		EventPollElement(const int _fd, const int _filter) : fd(_fd), filter(_filter) {}

		int fd = -1;
		int filter = 0;
	};
}
#if defined HAVE_KEVENT
#	include "PollManager_kqueue.h"
#elif defined HAVE_EPOLL
#	include "PollManager_epoll.h"
#else
#	include "PollManager_select.h"
#endif

#endif /* defined(__TFFIXEngine__TFEventPollManager__) */
