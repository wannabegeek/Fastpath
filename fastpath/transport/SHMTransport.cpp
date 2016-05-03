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

#include <fastpath/messages/MessageCodec.h>
#include "fastpath/transport/SHMTransport.h"
#include "fastpath/transport/TransportIOEvent.h"
#include "fastpath/transport/BackoffStrategy.h"
#include "fastpath/SharedMemoryBuffer.h"
#include "fastpath/transport/sm/SharedMemoryManager.h"
#include "fastpath/transport/sm/InterprocessNotifierClient.h"

namespace fp {
    SHMTransport::SHMTransport(const char *url_ptr, const char *description) : SHMTransport(url(url_ptr), description) {
    }

    SHMTransport::SHMTransport(const url &url, const char *description) : Transport(description), m_notifier(std::make_unique<InterprocessNotifierClient>()) {
        if (m_notifier->connect()) {
            m_smmanager = std::make_unique<SharedMemoryManager>("fprouter");
            m_buffer = std::make_unique<SharedMemoryBuffer>("ServerQueue", *m_smmanager);
        } else {
            m_connectionAttemptInProgress = std::async(std::launch::async, &SHMTransport::__connect, this);
        }
//        m_peer->setConnectionStateHandler([&](bool connected) {
//            DEBUG_LOG("Transport connected: " << std::boolalpha << connected);
//            if (m_notificationHandler) {
//                m_notificationHandler(connected ? CONNECTED : DISCONNECTED, "");
//            }
//            if (!connected && !m_shouldDisconnect) {
//                this->__connect();
//            }
//        });
    }

    SHMTransport::~SHMTransport() noexcept {
        this->__disconnect();
    }

    bool SHMTransport::__connect() noexcept {
        BackoffStrategy strategy;
        while (!m_notifier->is_connected() && m_shouldDisconnect == false) {
            if (!m_notifier->connect()) {
                DEBUG_LOG("Failed to connect trying again");
                strategy.backoff();
            }
        }

        m_smmanager = std::make_unique<SharedMemoryManager>("fprouter");
        m_buffer = std::make_unique<SharedMemoryBuffer>("ServerQueue", *m_smmanager);
        DEBUG_LOG("Either connected or given up");
        return true;
    }

    bool SHMTransport::__disconnect() noexcept {
        m_shouldDisconnect = true;
        m_notifier = std::make_unique<InterprocessNotifierClient>();
        m_buffer = nullptr;
        m_smmanager = nullptr;

        return true;
    }

    status SHMTransport::sendMessage(const Message &msg) noexcept {
        if (m_notifier->is_connected()) {
            SharedMemoryBuffer::mutable_storage_type storage(2048, m_smmanager->allocator());
            MessageCodec::encode(&msg, storage);

            m_buffer->notify(&storage);
            if (m_notifier->notify()) {
                return OK;
            }
        }
        return CANNOT_SEND;
    }

    status SHMTransport::sendMessageWithResponse(const Message &request, Message &reply, std::chrono::duration<std::chrono::milliseconds> &timeout) noexcept {
        return OK;
    }

    status SHMTransport::sendReply(const Message &reply, const Message &request) noexcept {
        return OK;
    }

    const bool SHMTransport::valid() const noexcept {
        return m_notifier->is_connected();
    }

    std::unique_ptr<TransportIOEvent> SHMTransport::createReceiverEvent(const std::function<void(const Transport *, MessageType &)> &messageCallback) {

//        return std::make_unique<TransportIOEvent>(m_peer->getSocket(), EventType::READ, [&, messageCallback](TransportIOEvent *event, const EventType type) {
//            static const size_t MTU_SIZE = 1500;
//
//            ssize_t size = 0;
//            while (true) {
//
//            }
//        }

        return nullptr;
    }
}


