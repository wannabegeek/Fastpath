/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 26/03/2016

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

#include "tcp_server_transport.h"
#include "fastpath/event/Queue.h"
#include "fastpath/event/DataEvent.h"
#include "fastpath/router/tcp_peer_connection.h"

namespace fp {
    tcp_server_transport::tcp_server_transport(Queue *queue, const ConnectionCallback &connectionCallback, const std::string &interface, const std::string &service) noexcept
            : server_transport(queue), m_server(interface, service) {

        if (m_server.connect(SocketOptionsDisableNagle | SocketOptionsDisableSigPipe | SocketOptionsNonBlocking)) {
            m_connectionAttempt = m_queue->registerEvent(m_server.getSocket(), EventType::READ, [&](DataEvent *event, const EventType eventType) {
                INFO_LOG("Someone has tried to connect");
                connectionCallback(this, std::unique_ptr<peer_connection>(new tcp_peer_connection(m_queue, m_server.acceptPendingConnection())));
            });
        }
        INFO_LOG("Waiting for TCP connections on " << interface << ":" << service);
    }

    tcp_server_transport::~tcp_server_transport() noexcept {
        m_server.disconnect();
        if (m_connectionAttempt != nullptr) {
            m_queue->unregisterEvent(m_connectionAttempt);
        }
    }
}