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

#ifndef FASTPATH_INTERPROCESSNOTIFIER_H
#define FASTPATH_INTERPROCESSNOTIFIER_H

#include <fastpath/transport/socket/UnixSocket.h>

#define READ_FD 0
#define WRITE_FD 1

namespace fp {
    class UnixSocket;

    class InterprocessNotifier {
    protected:
        std::unique_ptr<UnixSocket> m_socket;

    public:
        InterprocessNotifier(std::unique_ptr<UnixSocket> &&socket);
        virtual ~InterprocessNotifier() = default;

        bool send_fds(const int *fds, const size_t num_fds) noexcept;
        bool receive_fd(UnixSocket *socket, int *fd, size_t &num_fds, int &pid) noexcept;
    };
}

#endif //FASTPATH_INTERPROCESSNOTIFIER_H
