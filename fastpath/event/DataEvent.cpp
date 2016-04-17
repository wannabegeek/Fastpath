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

#include "fastpath/event/DataEvent.h"

#include "fastpath/event/Queue.h"

#include <cassert>

namespace DCF {

    void DataEvent::dispatch(DataEvent *event, const EventType &eventType) noexcept {
        this->__popDispatch();
        if (tf::likely(!m_pendingRemoval)) {
            m_callback(event, eventType);
        }
    }

    DataEvent::DataEvent(Queue *queue, const int fd, const EventType eventType, const std::function<void(DataEvent *, const EventType)> &callback)
            : IOEvent(queue, fd, eventType), m_callback(callback) {
    }

    DataEvent::DataEvent(DataEvent &&other) noexcept : IOEvent(std::move(other)), m_callback(std::move(other.m_callback)) {
    }

    const bool DataEvent::isEqual(const Event &other) const noexcept {
        return IOEvent::isEqual(other);
    }

    const bool DataEvent::__notify(const EventType &eventType) noexcept {
        assert(m_queue != nullptr);
        this->__pushDispatch();
        return m_queue->__enqueue(QueueElement(this, std::bind(&DataEvent::dispatch, this, this, eventType)));
    }

    void DataEvent::__destroy() noexcept {
        m_queue->unregisterEvent(this);
    }
}