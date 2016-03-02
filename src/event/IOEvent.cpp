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

//#include "SigIO.h"
//#include "SigIOCallback.h"
//#include "DCSession.h"
//
//SigIO::SigIO()
//{
//	m_isValid = DC_FALSE;
//	m_closure = NULL;
//}
//
//SigIO::~SigIO()
//{
//	if( m_isValid == DC_TRUE )
//		destroy();
//
//}
//
//DCStatus SigIO::create( DCQueue *queue, SigIOCallback *callback, DCEVENTHANDLER fileDesc, int registerFor, void *closure )
//{
//	if( m_isValid == DC_TRUE )
//		return DCStatus( DC_CANNOT_CREATE );
//	if( queue == NULL )
//		return DCStatus( DC_INVALID_QUEUE );
//	if( callback == NULL )
//		return DCStatus( DC_INVALID_CALLBACK );
//
//
//	m_closure = closure;
//	m_queue = queue;
//	m_callback = callback;
//	m_registerFor = registerFor;
//
//	openFd( fileDesc );
//	if( ( m_registerFor & IO_READ ) == IO_READ )
//	{
//		registerRead();
//	}
//	if( ( m_registerFor & IO_WRITE ) == IO_WRITE )
//	{
//		registerWrite();
//	}
//	// need to trigger the other thread to say there is
//	// more events to register
//	DCSession::signalEvent();
//
//	m_isValid = DC_TRUE;
//	return DCStatus( DC_OK );
//}
//
//DCStatus SigIO::destroy()
//{
//	if( m_isValid == DC_FALSE || m_inCallback == DC_TRUE )
//		return DCStatus( DC_CANNOT_DESTROY );
//
//	if( ( m_registerFor & IO_READ ) == IO_READ )
//	{
//		unregisterRead();
//	}
//	if( ( m_registerFor & IO_WRITE ) == IO_WRITE )
//	{
//		unregisterWrite();
//	}
//	// need to trigger the other thread to say there is
//	// more events to register
//	DCSession::signalEvent();
//
//	m_isValid = DC_FALSE;
//	return DCStatus( DC_OK );
//}
//
//void SigIO::readCallback()
//{
//	m_queue->add( this );
//}
//
//void SigIO::writeCallback()
//{
//	m_queue->add( this );
//}
//
//void SigIO::onEvent()
//{
//	m_inCallback = DC_TRUE;
//	if( ( m_registerFor & IO_READ ) == IO_READ )
//	{
//		m_callback->readIOCallback( this );
//	}
//	if( ( m_registerFor & IO_WRITE ) == IO_WRITE )
//	{
//		m_callback->writeIOCallback( this );
//	}
//	m_inCallback = DC_FALSE;
//}
//
//dc_bool SigIO::isValid() const
//{
//	return m_isValid;
//}

