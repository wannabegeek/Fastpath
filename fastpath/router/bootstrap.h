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

#ifndef TFDCF_BOOTSTRAP_H
#define TFDCF_BOOTSTRAP_H

#include <iosfwd>
#include <memory>

#include "fastpath/transport/TCPSocketServer.h"
#include "fastpath/event/InlineQueue.h"
#include "fastpath/router/peer_connection.h"

namespace fp{
    class bootstrap {
    private:
        DCF::InlineQueue m_dispatchQueue;

        DCF::TCPSocketServer m_server;
        bool m_shutdown = false;

        std::vector<std::unique_ptr<peer_connection>> m_connections;

        void message_handler(peer_connection *source, const subject<> &subject, const DCF::MessageBuffer::ByteStorageType &msgData) noexcept;
        void disconnection_handler(peer_connection *connection) noexcept;
    public:
        bootstrap(const std::string &interface, const std::string &service);
        ~bootstrap();

        void run();
    };
}

#endif //TFDCF_BOOTSTRAP_H
