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

#ifndef FASTPATH_INTERPROCESSNOTIFIERSERVER_H
#define FASTPATH_INTERPROCESSNOTIFIERSERVER_H

#include <fastpath/event/EventType.h>
#include <fastpath/transport/TransportIOEvent.h>
#include "InterprocessNotifier.h"
#include "fastpath/event/notifier.h"

namespace fp {
    class InterprocessNotifierServer : public InterprocessNotifier {
    private:
        std::function<void(std::unique_ptr<fp::notifier> &&notifier)> m_callback;
    public:
        InterprocessNotifierServer(std::function<void(std::unique_ptr<fp::notifier> &&notifier)> callback);

        virtual std::unique_ptr<TransportIOEvent> createReceiverEvent();
    };
}


#endif //FASTPATH_INTERPROCESSNOTIFIERSERVER_H
