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

#ifndef TFDCF_UNIXSOCKET_H
#define TFDCF_UNIXSOCKET_H

#include <sys/un.h>

#include "fastpath/transport/Socket.h"

namespace DCF {
    class UnixSocket : public Socket{
    protected:
        struct sockaddr_un m_addr;

    public:

        UnixSocket(const std::string &path) throw(socket_error);
        UnixSocket(const int socketFd, const bool connected) noexcept;

        UnixSocket(UnixSocket &&other) noexcept;

        virtual ~UnixSocket() noexcept;

        void setOptions(int options) noexcept override;

        bool send_ancillary(const struct msghdr *msg, int flags) noexcept;
        const Socket::ReadResult read_ancillary(struct msghdr *msg) noexcept;
    };
}


#endif //TFDCF_UNIXSOCKET_H
