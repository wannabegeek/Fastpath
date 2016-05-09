/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 09/05/2016

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

#ifndef FASTPATH_SIGNALEVENT_H
#define FASTPATH_SIGNALEVENT_H

#include <chrono>

#include "fastpath/event/Event.h"
#include "fastpath/status.h"

namespace fp {

    class Queue;

    class SignalEvent final : public Event {
        friend class EventManager;

    private:
        int m_identifier = -1;

        std::function<void(SignalEvent *, int)> m_callback;
        const int m_signal = -1;

        void dispatch(SignalEvent *event) noexcept;

    public:
        SignalEvent(Queue *queue, const int signal, const std::function<void(SignalEvent *, int)> &callback);
        SignalEvent(SignalEvent &&other) noexcept;

        const int signal() const noexcept { return m_signal; }
        const int identifier() const noexcept { return m_identifier; }

        const bool isEqual(const Event &other) const noexcept override;
        const bool __notify(const EventType &eventType) noexcept override;
        void __destroy() noexcept override;
    };
}

#endif
