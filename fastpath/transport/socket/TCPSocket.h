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

#ifndef FASTPATH_TCPSOCKET_H
#define FASTPATH_TCPSOCKET_H

#include "Socket.h"

namespace fp {
    class TCPSocket : public Socket {
    protected:
        uint16_t m_port;
        struct hostent m_host;
        struct addrinfo *m_hostInfo = NULL;

    public:
        TCPSocket(const std::string &host, const std::string &service) throw(socket_error);
        TCPSocket(const std::string &host, const uint16_t &port) throw(socket_error);
        TCPSocket(const struct hostent *host, const uint16_t &port);
        TCPSocket(const int socketFd, const bool connected);

        virtual ~TCPSocket();

        void setOptions(int options) noexcept override;
    };
}


#endif //FASTPATH_TCPSOCKET_H
