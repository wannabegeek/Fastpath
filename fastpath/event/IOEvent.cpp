/***************************************************************************
                          IOEvent.cpp
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

#include "IOEvent.h"

namespace DCF {
    IOEvent::IOEvent(Queue *queue, const int fd, const EventType eventType) noexcept
            : Event(queue), m_fd(fd), m_eventTypes(eventType) {
    }

    IOEvent::IOEvent(IOEvent &&other) noexcept : Event(std::move(other)), m_fd(other.m_fd), m_eventTypes(other.m_eventTypes) {
    }

    const bool IOEvent::isEqual(const Event &other) const noexcept {
        if (typeid(other) == typeid(IOEvent)) {
            const IOEvent &f = static_cast<const IOEvent &>(other);
            return m_fd == f.m_fd && m_eventTypes == f.m_eventTypes;
        }
        return false;
    }
}
