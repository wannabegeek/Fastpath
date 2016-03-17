/***************************************************************************
                          Timer.cpp
                          -------------------
    copyright            : (C) 2004 by Tom Fewster
    email                : tom@wannabegeek.com
    version              : $Revision: 1.19 $
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

#include "TimerEvent.h"
//#include "DCQueue.h"
//#include "DCSession.h"

//TimerEvent::TimerEvent()
//{
//	m_isValid = false;
//}
//
//TimerEvent::~TimerEvent()
//{
//	if( m_isValid == DC_TRUE )
//		destroy();
//}
//
//status TimerEvent::create(DCQueue *queue, TimerCallback *callback, dc_u32 period)
//{
//	if( m_isValid == DC_TRUE )
//		return DCStatus( DC_CANNOT_CREATE );
//
//	if( queue == NULL )
//		return DCStatus( DC_INVALID_QUEUE );
//	if( callback == NULL )
//		return DCStatus( DC_INVALID_CALLBACK );
//
//	m_closure = closure;
//	m_queue = queue;
//	m_callback = callback;
//
//#ifdef _WIN32
//	long sec = (long)(period/1000);
//	long usec = period - sec*1000;
//#else
//	period *= 1000;
//	m_period = period;
//	long sec = (long)(period/1000000);
//	long usec = period - sec*1000000;
//#endif
//
//	registerTimer( sec, usec );
//	// need to trigger the other thread to say there is
//	// more events to register
//	DCSession::signalEvent();
//
//	m_isValid = DC_TRUE;
//	return DCStatus( DC_OK );
//}
//
//DCStatus TimerEvent::destroy()
//{
//	if( m_isValid == DC_FALSE || m_inCallback == DC_TRUE )
//		return DCStatus( DC_CANNOT_DESTROY );
//
//	unregisterTimer();
//	// need to trigger the other thread to say there is
//	// more events to register
//	DCSession::signalEvent();
//
//	m_isValid = DC_FALSE;
//	return DCStatus( DC_OK );
//}
//
//void TimerEvent::timerCallback()
//{
//	m_queue->add( this );
//}
//
//void TimerEvent::onEvent()
//{
//	m_inCallback = DC_TRUE;
//	m_callback->timerCallback( this );
//	m_inCallback = DC_FALSE;
//}
//
//dc_bool TimerEvent::isValid() const
//{
//	return m_isValid;
//}
