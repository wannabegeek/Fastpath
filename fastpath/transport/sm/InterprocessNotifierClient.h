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

#ifndef FASTPATH_INTERPROCESSNOTIFIERCLIENT_H
#define FASTPATH_INTERPROCESSNOTIFIERCLIENT_H

#include "InterprocessNotifier.h"
#include "fastpath/event/notifier.h"

namespace fp {
    class InterprocessNotifierClient : public InterprocessNotifier {
    private:
        fp::notifier inbound_notification;
        fp::notifier outbound_notification;

        bool m_connected = false;
    public:
        InterprocessNotifierClient() noexcept;

        inline bool is_connected() const noexcept { return m_connected; }

        bool connect() noexcept;
        bool notify() noexcept;

        int signal_fd() const noexcept { return inbound_notification.read_handle() ; }
    };
}

#endif //FASTPATH_INTERPROCESSNOTIFIERCLIENT_H
