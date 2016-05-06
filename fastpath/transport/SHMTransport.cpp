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

#include <cstring>

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

    SHMTransport::SHMTransport(const url &url, const char *description) : Transport(description), m_notifier(std::make_unique<InterprocessNotifierClient>()), m_url(url) {

        auto on_connect = [&, this]() {
            const std::string sm_name = "fprouter_" + this->m_url.port();
            m_smmanager = std::make_unique<SharedMemoryManager>(sm_name.c_str());
            m_sendQueue = std::make_unique<SharedMemoryBuffer>("ServerQueue", *m_smmanager);

            char queueName[32];
            ::sprintf(queueName, "ClientQueue_%i", ::getpid());
            m_recvQueue = std::make_unique<SharedMemoryBuffer>(queueName, *m_smmanager);
            m_connected = true;
        };

        if (m_notifier->connect()) {
            on_connect();
        } else {
            m_connectionAttemptInProgress = std::async(std::launch::async, &SHMTransport::__connect, this, on_connect);
        }

//        m_notifier->setConnectionStateHandler([&](bool connected) {
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

    bool SHMTransport::__connect(std::function<void()> on_connect) noexcept {
        BackoffStrategy strategy;
        while (!m_notifier->is_connected() && m_shouldDisconnect == false) {
            if (!m_notifier->connect()) {
                DEBUG_LOG("Failed to connect trying again");
                strategy.backoff();
            }
        }

        if (!m_shouldDisconnect) {
            on_connect();
        }
        DEBUG_LOG("Either connected or given up");
        return true;
    }

    bool SHMTransport::__disconnect() noexcept {
        m_connected = false;
        m_shouldDisconnect = true;
        m_notifier = std::make_unique<InterprocessNotifierClient>();
        m_recvQueue = nullptr;
        m_sendQueue = nullptr;
        m_smmanager = nullptr;

        return true;
    }

    status SHMTransport::sendMessage(const Message &msg) noexcept {
        if (m_connected.load(std::memory_order_relaxed) && m_notifier->is_connected()) {
            SharedMemoryBuffer::mutable_storage_type storage(2048, m_smmanager->allocator());
            MessageCodec::encode(&msg, storage);

            m_sendQueue->notify(&storage);
            if (m_notifier->notify()) {
                return OK;
            } else {
                ERROR_LOG("Failed to notify counterparty");
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
        return m_notifier->is_connected() && m_connected.load(std::memory_order_relaxed);
    }

    std::unique_ptr<TransportIOEvent> SHMTransport::createReceiverEvent(const std::function<void(const Transport *, MessageType &)> &messageCallback) {
        INFO_LOG("Registering callback on: " << m_notifier->signal_fd());
        return std::make_unique<TransportIOEvent>(m_notifier->signal_fd(), EventType::READ, [&, messageCallback](TransportIOEvent *event, const EventType type) {
            m_recvQueue->retrieve([&](auto &ptr) {
                MessagePoolType::shared_ptr_type message = m_msg_pool.allocate_shared_ptr();

                if (MessageCodec::decode(message.get(), ptr)) {
                    messageCallback(this, message);
                } else {
                    message = nullptr;
                }

                // if ref-counter == 0, deallocate the shared memory
            });
        });
    }
}


