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

#ifndef FASTPATH_SHMTRANSPORT_H
#define FASTPATH_SHMTRANSPORT_H

#include <future>

#include "fastpath/status.h"
#include "fastpath/messages/Message.h"
#include "fastpath/transport/URL.h"
#include "fastpath/transport/Transport.h"

namespace fp {
    class SharedMemoryManager;
    class SharedMemoryBuffer;
    class InterprocessNotifierClient;

    class SHMTransport : public Transport {
    private:
        virtual std::unique_ptr<TransportIOEvent> createReceiverEvent(const std::function<void(const Transport *, MessageType &)> &messageCallback) override;

        std::unique_ptr<SharedMemoryManager> m_smmanager;
        std::unique_ptr<SharedMemoryBuffer> m_sendQueue;
        std::unique_ptr<SharedMemoryBuffer> m_recvQueue;
        std::unique_ptr<InterprocessNotifierClient> m_notifier;

        std::atomic<bool> m_connected = ATOMIC_VAR_INIT(false);
        std::atomic<bool> m_shouldDisconnect;
        std::future<bool> m_connectionAttemptInProgress;

        const url m_url;

        bool __connect(std::function<void()> on_connect) noexcept;
        bool __disconnect() noexcept;
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

#endif //FASTPATH_SHMTRANSPORT_H
