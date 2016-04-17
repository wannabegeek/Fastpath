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

#include "fastpath/transport/UnixSocketClient.h"
#include "fastpath/utils/logger.h"

namespace DCF {
    UnixSocketClient::UnixSocketClient(const std::string &path) : UnixSocket(path) {
    }

    UnixSocketClient::UnixSocketClient(UnixSocketClient &&other) noexcept : UnixSocket(std::move(other)) {
    }

    bool UnixSocketClient::connect(SocketOptions options) noexcept {
        if (::connect(m_socket, reinterpret_cast<struct sockaddr *>(&m_addr), sizeof(struct sockaddr_un)) != 0) {
            INFO_LOG("Failed to connect to IPC endpoint");
            return false;
        }

        m_connected = true;
        return true;
    }
}
