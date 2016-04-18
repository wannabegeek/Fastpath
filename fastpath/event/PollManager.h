/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef __TFFIXEngine__TFEventPollManager__
#define __TFFIXEngine__TFEventPollManager__

#include <chrono>
#include <functional>

#include "fastpath/config.h"

namespace fp {
	using DistantFutureType = std::chrono::microseconds;
	static constexpr DistantFutureType DistantFuture = DistantFutureType::max();

	struct EventPollIOElement {
		EventPollIOElement() noexcept {}
		EventPollIOElement(const int _fd, const int _filter) noexcept : identifier(_fd), filter(_filter) {}

		int identifier = -1;
		int filter = 0;
	};

	struct EventPollTimerElement {
        EventPollTimerElement() noexcept {}
        EventPollTimerElement(const int _fd, const std::chrono::microseconds _timeout) noexcept : identifier(_fd), timeout(_timeout) {}
        EventPollTimerElement(const int _fd) noexcept : identifier(_fd) {}

		int identifier = -1;
        std::chrono::microseconds timeout;
	};

    class EventPoll {
    private:
        int m_fd = -1;
        int m_events = 0;
        int m_err_count = 0;

        static constexpr size_t maxDispatchSize = 256;
//        static constexpr const bool greater_than(const size_t x, const size_t y) { return x >= y; }
//        static_assert(greater_than(MAX_EVENTS, maxDispatchSize), "MAX_EVENTS template parameter not large enough");

    public:
        EventPoll();
        ~EventPoll();

        bool add(const EventPollTimerElement &event) noexcept;
        bool update(const EventPollTimerElement &event) noexcept;
        bool remove(const EventPollTimerElement &event) noexcept;

        bool add(const EventPollIOElement &event) noexcept;
        bool remove(const EventPollIOElement &event) noexcept;

        int run(std::function<void(EventPollIOElement &&)> io_events, std::function<void(EventPollTimerElement &&)> timer_events) noexcept;
    };
}

#endif /* defined(__TFFIXEngine__TFEventPollManager__) */
