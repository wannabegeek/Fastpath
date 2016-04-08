/***************************************************************************
                          SigIO.cpp
                          -------------------
    copyright            : (C) 2004 by Tom Fewster
    email                : tom@wannabegeek.com
    version              : $Revision: 1.17 $
    date                 : $Date: 2004/03/04 08:44:13 $

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
    IOEvent::IOEvent(Queue *queue, const int fd, const EventType eventType)
            : Event(queue), m_fd(fd), m_eventTypes(eventType) {
    }

    IOEvent::IOEvent(IOEvent &&other) noexcept : Event(std::move(other)), m_fd(other.m_fd), m_eventTypes(other.m_eventTypes) {
    }

    const bool IOEvent::isEqual(const Event &other) const noexcept {
        try {
            const IOEvent &f = dynamic_cast<const IOEvent &>(other);
            return m_fd == f.m_fd && m_eventTypes == f.m_eventTypes;
        } catch (const std::bad_cast &e) {
            return false;
        }
    }
}
