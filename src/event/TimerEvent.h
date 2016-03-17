/***************************************************************************
                          Timer.h
                          -------------------
    copyright            : (C) 2004 by Tom Fewster
    email                : tom@wannabegeek.com
    version              : $Revision: 1.14 $
    date                 : $Date: 2004/02/10 14:24:36 $

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

#ifndef TIMER_H
#define TIMER_H

class TimerCallback;
class DCQueue;

#include <chrono>
#include "Event.h"
#include "Session.h"
#include "status.h"

namespace  DCF {

    class Queue;

	class TimerEvent final : public Event {
        friend class EventManager;

//    private:
    public:
        typedef enum {
			TIMEOUTSTATE_NONE,
			TIMEOUTSTATE_START,
			TIMEOUTSTATE_PROGRESS
		} TimeoutState;

        mutable TimeoutState m_timeoutState = TIMEOUTSTATE_NONE;
		std::chrono::microseconds m_timeout;
		// TODO: this needs to be atomic, since it can be updated by the event loop and the client side
		mutable std::chrono::microseconds m_timeLeft;
		mutable std::chrono::steady_clock::time_point m_lastTime;
		bool pendingRemoval = false;

        std::function<void(TimerEvent *)> m_callback;

        void dispatch(TimerEvent *event) {
            this->__setAwaitingDispatch(false);
            if (m_active) {
                m_callback(event);
            }
		}

    public:
        TimerEvent() : m_callback(nullptr) {
        }

		TimerEvent(const std::chrono::milliseconds &timeout, const std::function<void(TimerEvent *)> &callback)
				: m_timeoutState(TIMEOUTSTATE_START), m_timeout(timeout), m_timeLeft(timeout), m_callback(callback) {
		}

		TimerEvent(TimerEvent &&other) : Event(std::move(other)), m_timeout(std::move(other.m_timeout)), m_callback(std::move(other.m_callback)) {
		}

//		TimerEvent(Queue *queue, const uint64_t &timeout, const std::function<void(const TimerEvent *)> &callback)
//				: Event(queue), m_timeoutState(TIMEOUTSTATE_START), m_timeout(timeout), m_timeLeft(timeout), m_callback(callback) {
//			m_queue->__registerEvent(*this);
//		}

		~TimerEvent() {
			this->destroy();
        }

        status create(const std::chrono::milliseconds &timeout, const std::function<void(TimerEvent *)> &callback) {
            if (!m_active) {
                m_timeoutState = TIMEOUTSTATE_START;
                m_timeout = timeout;
                m_timeLeft = timeout;
                m_callback = callback;
                m_active = true;
                return OK;
            }

            return ALREADY_ACTIVE;
        }

        status destroy() {
			if (m_active) {
				m_active = false;
				if (m_isRegistered.load()) {
					m_queue->__unregisterEvent(*this);
				}
			} else {
				return CANNOT_DESTROY;
			}
			return OK;
        }

        void reset() {
            m_timeLeft = m_timeout;
            if (m_isRegistered.load()) {
                m_queue->__notifyEventManager();
            }
		}

		void setTimeout(const std::chrono::microseconds &timeout) {
			m_timeout = timeout;
			this->reset();
		}

		const bool isEqual(const Event &other) const noexcept override {
			try {
				const TimerEvent &f = dynamic_cast<const TimerEvent &>(other);
				return m_timeout == f.m_timeout;
			} catch (const std::bad_cast &e) {
				return false;
			}
		}

        const bool __notify(const EventType &eventType) noexcept override {
            assert( m_queue != nullptr);
            std::function<void ()> dispatcher = std::bind(&TimerEvent::dispatch, this, this);
            return m_queue->__enqueue(dispatcher);
        };
    };
}

#endif
