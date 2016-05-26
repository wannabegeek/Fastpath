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

#ifndef FASTPATH_TCP_SERVER_TRANSPORT_H
#define FASTPATH_TCP_SERVER_TRANSPORT_H

#include "fastpath/router/server_transport.h"
#include "fastpath/transport/socket/TCPSocketServer.h"

namespace fp {
    class DataEvent;
    class tcp_peer_connection;

    class tcp_server_transport : public server_transport {
        TCPSocketServer m_server;
        DataEvent *m_connectionAttempt = nullptr;
    public:

        tcp_server_transport(Queue *queue, const ConnectionCallback &connectionCallback, const std::string &interface, const std::string &service) noexcept;
        ~tcp_server_transport() noexcept;
    };
}

#endif //FASTPATH_TCP_SERVER_TRANSPORT_H
