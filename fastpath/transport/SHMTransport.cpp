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

#include "fastpath/transport/SHMTransport.h"
#include "fastpath/transport/TransportIOEvent.h"

namespace fp {
    SHMTransport::SHMTransport(const char *url_ptr, const char *description) : SHMTransport(url(url_ptr), description) {
    }

    SHMTransport::SHMTransport(const url &url, const char *description) : Transport(description) {

    }

    SHMTransport::~SHMTransport() noexcept {
    }

    status SHMTransport::sendMessage(const Message &msg) noexcept {
        return OK;
    }

    status SHMTransport::sendMessageWithResponse(const Message &request, Message &reply, std::chrono::duration<std::chrono::milliseconds> &timeout) noexcept {
        return OK;
    }

    status SHMTransport::sendReply(const Message &reply, const Message &request) noexcept {
        return OK;
    }

    const bool SHMTransport::valid() const noexcept {
        return false;
    }

    std::unique_ptr<TransportIOEvent> SHMTransport::createReceiverEvent(const std::function<void(const Transport *, MessageType &)> &messageCallback) {
        return nullptr;
    }
}


