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

#ifndef FASTPATH_SERVER_TRANSPORT_H
#define FASTPATH_SERVER_TRANSPORT_H

#include "fastpath/router/peer_connection.h"

namespace fp {
    class Queue;

    class server_transport {
    protected:
        Queue *m_queue;
    public:
        typedef std::function<void(server_transport *, std::unique_ptr<peer_connection> &&)> ConnectionCallback;

        server_transport(Queue *queue) noexcept : m_queue(queue) {}
        virtual ~server_transport() noexcept = default;
    };
}

#endif //FASTPATH_SERVERTRANSPORT_H
