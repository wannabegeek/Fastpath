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

#include <future>
#include <chrono>
#include "fastpath/utils/logger.h"

#include "TCPTransport.h"
#include "fastpath/transport/socket/TCPSocketClient.h"
#include "fastpath/messages/Message.h"
#include "fastpath/messages/MessageCodec.h"
#include "fastpath/transport/TransportIOEvent.h"

namespace fp {

    TCPTransport::TCPTransport(const char *url_ptr, const char *description) : TCPTransport(url(url_ptr), description) {
    }

    TCPTransport::TCPTransport(const url &url, const char *description) : Transport(description), m_url(url), m_shouldDisconnect(false), m_sendBuffer(1024), m_readBuffer(1500) {
        DEBUG_LOG("Connecting to: " << m_url);
        m_peer = std::make_unique<TCPSocketClient>(m_url.host(), m_url.port());

        if (!m_peer->connect()) {
            m_connectionAttemptInProgress = std::async(std::launch::async, &TCPTransport::__connect, this);
        }
        m_peer->setConnectionStateHandler([&](bool connected) {
            DEBUG_LOG("Transport connected: " << std::boolalpha << connected);
            if (m_notificationHandler) {
                m_notificationHandler(connected ? CONNECTED : DISCONNECTED, "");
            }
            if (!connected && !m_shouldDisconnect) {
                this->__connect();
            }
        });
    }

    TCPTransport::~TCPTransport() noexcept {
        this->__disconnect();
    }

    bool TCPTransport::__connect() noexcept {
        BackoffStrategy strategy;
        while (!m_peer->isConnected() && m_shouldDisconnect == false) {
            if (!m_peer->connect()) {
                DEBUG_LOG("Failed to connect trying again");
                strategy.backoff();
            }
        }

        DEBUG_LOG("Either connected or given up");
        return true;
    }

    bool TCPTransport::__disconnect() noexcept {
        m_shouldDisconnect = true;
        if (m_peer->isConnected()) {
            return m_peer->disconnect();
        } else {
            if (m_connectionAttemptInProgress.valid()) {
                DEBUG_LOG("Shutting down our connection attempt loop");
                m_connectionAttemptInProgress.wait();
            }
        }

        return true;
    }

    status TCPTransport::sendMessage(const Message &msg) noexcept {
        if (m_peer->isConnected()) {
            fp::MessageCodec::encode(&msg, m_sendBuffer.mutableBuffer());
            const byte *data = nullptr;
            size_t len = m_sendBuffer.bytes(&data);
            if (m_peer->send(reinterpret_cast<const char *>(data), len)) {
                m_sendBuffer.erase_front(len);
                return OK;
            }
        }
        return CANNOT_SEND;
    }

    status TCPTransport::sendMessageWithResponse(const Message &request, Message &reply,
                                                         std::chrono::duration<std::chrono::milliseconds> &timeout) noexcept {
        return this->sendMessage(request);
    }

    status TCPTransport::sendReply(const Message &reply, const Message &request) noexcept {
        return this->sendMessage(reply);
    }

    const bool TCPTransport::valid() const noexcept {
        return m_peer->isConnected();
    }

    bool TCPTransport::processData(const fp::MessageBuffer::ByteStorageType &storage, const std::function<void(const Transport *, MessageType &)> &messageCallback) noexcept {
        size_t msg_length = 0;
        if (MessageCodec::have_complete_message(storage, msg_length) == MessageCodec::CompleteMessage) {
            MessagePoolType::shared_ptr_type message = m_msg_pool.allocate_shared_ptr();
            storage.mark();

            if (MessageCodec::decode(message.get(), storage)) {
                messageCallback(this, message);
                return true;
            } else {
                storage.resetRead();
                message = nullptr;
            }
        }

        return false;
    }

    std::unique_ptr<TransportIOEvent> TCPTransport::createReceiverEvent(const std::function<void(const Transport *, MessageType &)> &messageCallback) {
        return std::make_unique<TransportIOEvent>(m_peer->getSocket(), EventType::READ, [&, messageCallback](TransportIOEvent *event, const EventType type) {
            static const size_t MTU_SIZE = 1500;

            ssize_t size = 0;
            while (true) {
                fp::Socket::ReadResult result = m_peer->read(reinterpret_cast<const char *>(m_readBuffer.allocate(MTU_SIZE)), MTU_SIZE, size);
                m_readBuffer.erase_back(MTU_SIZE - size);

                if (result == fp::Socket::MoreData) {
                    const fp::MessageBuffer::ByteStorageType &storage = m_readBuffer.byteStorage();

                    while (this->processData(storage, messageCallback));

                    m_readBuffer.erase_front(storage.bytesRead());
                } else if (result == fp::Socket::NoData) {
                    break;
                } else if (result == fp::Socket::Closed) {
                    DEBUG_LOG("Client Socket closed");
                    break;
                }
            }
        });
    }
}
