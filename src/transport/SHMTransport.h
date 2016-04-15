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

#ifndef TFDCF_SHMTRANSPORT_H
#define TFDCF_SHMTRANSPORT_H

#include "status.h"
#include "messages/Message.h"
#include "URL.h"
#include "Transport.h"

namespace DCF {
    class SHMTransport : public Transport {
    private:
        virtual std::unique_ptr<TransportIOEvent> createReceiverEvent(const std::function<void(const Transport *, MessageType &)> &messageCallback) override;

    public:
        // This should have the format dcf://localhost:1234
        SHMTransport(const char *url, const char *description);
        SHMTransport(const url &url, const char *description);

        SHMTransport(const SHMTransport &) = delete;
        SHMTransport &operator=(const SHMTransport &) = delete;

        ~SHMTransport() noexcept;

        status sendMessage(const Message &msg) noexcept override;
        status sendMessageWithResponse(const Message &request, Message &reply, std::chrono::duration<std::chrono::milliseconds> &timeout) noexcept override;
        status sendReply(const Message &reply, const Message &request) noexcept override;
        const bool valid() const noexcept override;

    };
}

#endif //TFDCF_SHMTRANSPORT_H
