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

#ifndef IOEVENT_H
#define IOEVENT_H

#include "Event.h"
#include "EventType.h"

namespace DCF {

	class IOEvent : public Event {
        friend class EventManager;

	private:
		int m_fd;
		EventType m_eventTypes;

	public:

		IOEvent(Queue *queue, const int fd, const EventType eventType) noexcept;
		IOEvent(IOEvent &&other) noexcept;
		virtual ~IOEvent() noexcept {}

        inline const int fileDescriptor() const noexcept {
            return m_fd;
        }

		inline const EventType eventTypes() const noexcept {
			return m_eventTypes;
		}

		virtual const bool isEqual(const Event &other) const noexcept override;

	};
}

#endif
