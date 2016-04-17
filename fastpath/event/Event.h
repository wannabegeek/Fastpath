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

#ifndef TFDCF_EVENT_H
#define TFDCF_EVENT_H

#include <functional>
#include <atomic>

#include "fastpath/event/EventType.h"
#include "fastpath/event/Session.h"

namespace DCF {
    class Queue;

    class Event {
    protected:

        Queue *m_queue;

        bool m_pendingRemoval = false;
        std::atomic<uint16_t> m_awaitingDispatch = ATOMIC_VAR_INIT(0);

        virtual const bool isEqual(const Event &other) const noexcept = 0;

        inline void __pushDispatch() noexcept {
            m_awaitingDispatch++;
        }

        inline void __popDispatch() noexcept {
            m_awaitingDispatch--;
        }

    public:
        Event(Queue *queue) noexcept : m_queue(queue) {
        }

        Event(Event &&other) noexcept : m_queue(other.m_queue), m_pendingRemoval(other.m_pendingRemoval) {
        }

        Event(const Event &other) = delete;
        Event& operator=(Event const&) = delete;

        virtual ~Event() noexcept {}

        virtual const bool __notify(const EventType &eventType) noexcept = 0;
        virtual void __destroy() noexcept = 0;

        void __setPendingRemoval(const bool flag) noexcept {
            m_pendingRemoval = flag;
        }

        inline const bool __pendingRemoval() const noexcept {
            return m_pendingRemoval;
        }

        const bool __awaitingDispatch() const noexcept {
            return m_awaitingDispatch.load(std::memory_order_relaxed) != 0;
        }

        bool operator==(const Event &other) noexcept {
            return m_queue == other.m_queue
                    && this->isEqual(other);
        }
    };
}

#endif //TFDCF_EVENT_H
