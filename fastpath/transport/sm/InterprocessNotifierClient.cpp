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

#include <fastpath/transport/socket/UnixSocketClient.h>
#include <fastpath/utils/logger.h>
#include "InterprocessNotifierClient.h"

namespace fp {
    InterprocessNotifierClient::InterprocessNotifierClient() noexcept : InterprocessNotifier(std::make_unique<UnixSocketClient>("test_unix")) {
    }

    bool InterprocessNotifierClient::connect() noexcept {
        if (!m_connected && m_socket->connect(fp::SocketOptionsNone)) {
            int p[] = {outbound_notification.read_handle(), inbound_notification.signal_handle()};
            this->send_fds(p, 2);
            outbound_notification.close_handle(pipe_read);
            inbound_notification.close_handle(pipe_write);
            m_connected = true;
            return true;
        }

        return false;
    }

    bool InterprocessNotifierClient::notify() noexcept {
        if (!outbound_notification.notify()) {
            ERROR_LOG("Notify failed " << strerror(errno));
            m_connected = false;
        }

        return m_connected;
    }
}

