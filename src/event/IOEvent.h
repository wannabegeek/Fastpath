/***************************************************************************
                          SigIO.h
                          -------------------
    copyright            : (C) 2004 by Tom Fewster
    email                : tom@wannabegeek.com
    version              : $Revision: 1.12 $
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

#ifndef SIGIO_H
#define SIGIO_H


#include "Event.h"
#include "EventType.h"

namespace DCF {

    class Queue;

	class IOEvent final : public Event {
        friend class EventManager;

	private:
		int m_fd;
		EventType m_eventTypes;
		bool m_pendingRemoval = false;

		std::function<void(IOEvent *, const EventType)> m_callback;

        void dispatch(IOEvent *event, const EventType &eventType) {
            this->__setAwaitingDispatch(false);
            if (m_active) {
                m_callback(event, eventType);
            }
        }

	public:
        IOEvent() : m_fd(-1), m_callback(nullptr) {}

		IOEvent(Queue *queue, const int fd, const EventType eventType, const std::function<void(IOEvent *, const EventType)> &callback)
				: Event(queue), m_fd(fd), m_eventTypes(eventType), m_callback(callback) {
            m_queue->__registerEvent(*this);
		};

		IOEvent(IOEvent &&other) : Event(std::move(other)), m_fd(other.m_fd), m_eventTypes(other.m_eventTypes), m_pendingRemoval(other.m_pendingRemoval), m_callback(std::move(other.m_callback)) {
		}

		~IOEvent() {
            if (m_active) {
                m_queue->__unregisterEvent(*this);
            }
        }

        void registerEvent(Queue *queue, const int fd, const EventType eventType, const std::function<void(IOEvent *, const EventType)> &callback) {
            setQueue(queue);
            m_fd = fd;
            m_eventTypes = eventType;
            m_callback = callback;
            m_queue->__registerEvent(*this);
            m_active = true;
        }

        void unregisterEvent() {
            if (m_active && m_isRegistered.load()) {
                m_active = false;
                m_queue->__unregisterEvent(*this);
            }
        }

        inline const int fileDescriptor() const noexcept {
            return m_fd;
        }

		inline const EventType eventTypes() const noexcept {
			return m_eventTypes;
		}

		const bool isEqual(const Event &other) const noexcept override {
			try {
				const IOEvent &f = dynamic_cast<const IOEvent &>(other);
				return m_fd == f.m_fd && m_eventTypes == f.m_eventTypes;
			} catch (const std::bad_cast &e) {
				return false;
			}
		}

        const bool __notify(const EventType &eventType) noexcept override {
            std::function<void ()> dispatcher = std::bind(&IOEvent::dispatch, this, this, eventType);
            return m_queue->__enqueue(dispatcher);
        };
    };
}

//class SigIOCallback;
//class DCQueue;
//
//#include "EventHandler.h"
//#include "DCEvent.h"
//#include <DCStatus.h>
//
//#include <Win32DLL.h>
//
//class SigIO : public EventHandler, public DCEvent
//{
//	public:
//		WIN32DLL SigIO();
//		WIN32DLL ~SigIO();
//
//		WIN32DLL DCStatus create( DCQueue *queue, SigIOCallback *callback, DCEVENTHANDLER fileDesc, int registerFor, void *closure = NULL );
//		WIN32DLL DCStatus destroy();
//
//		void readCallback();
//		void writeCallback();
//		void onEvent();
//
//		WIN32DLL EventType getEventType() { return DC_SIGIO; }
//		WIN32DLL dc_bool isValid() const;
//	private:
//		DCQueue *m_queue;
//		SigIOCallback *m_callback;
//		int m_registerFor;
//
//		dc_bool m_isValid;
//};

#endif
