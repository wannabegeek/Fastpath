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

#ifndef TFDCF_TCPTRANSPORT_H
#define TFDCF_TCPTRANSPORT_H

#include <future>

#include "fastpath/transport/Transport.h"
#include "fastpath/transport/TCPSocketClient.h"
#include "fastpath/transport/URL.h"
#include "fastpath/MessageBuffer.h"

namespace DCF {

    class Event;
    class TransportIOEvent;

    class TCPTransport : public Transport {
    private:
        virtual std::unique_ptr<TransportIOEvent> createReceiverEvent(const std::function<void(const Transport *, MessageType &)> &messageCallback) override;
        bool processData(const DCF::MessageBuffer::ByteStorageType &storage, const std::function<void(const Transport *, MessageType &)> &messageCallback) noexcept;
    protected:
        std::unique_ptr<TCPSocketClient> m_peer;
        const url m_url;

        std::atomic<bool> m_shouldDisconnect;
        std::future<bool> m_connectionAttemptInProgress;

        MessageBuffer m_sendBuffer;
        MessageBuffer m_readBuffer;

        bool __connect() noexcept;
        bool __disconnect() noexcept;
    public:
        // This should have the format dcf://localhost:1234
        TCPTransport(const char *url, const char *description);
        TCPTransport(const url &url, const char *description);

        TCPTransport(const TCPTransport &) = delete;
        TCPTransport &operator=(const TCPTransport &) = delete;

        ~TCPTransport() noexcept;

        status sendMessage(const Message &msg) noexcept override;
        status sendMessageWithResponse(const Message &request, Message &reply, std::chrono::duration<std::chrono::milliseconds> &timeout) noexcept override;
        status sendReply(const Message &reply, const Message &request) noexcept override;
        const bool valid() const noexcept override;

    };
}

#endif //TFDCF_TCPTRANSPORT_H
