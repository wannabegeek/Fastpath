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

#include "fastpath/event/Queue.h"
#include "fastpath/messages/Message.h"
#include "fastpath/router/tcp_peer_connection.h"
#include "fastpath/router/message_wrapper.h"
#include "fastpath/messages/subject.h"
#include "fastpath/transport/socket/Socket.h"

namespace fp {
    tcp_peer_connection::tcp_peer_connection(Queue *queue, std::unique_ptr<Socket> socket)
            : peer_connection(queue), m_socket(std::move(socket)), m_buffer(4500) {

        m_socketEvent = queue->registerEvent(m_socket->getSocket(), EventType::READ,
                             std::bind(&tcp_peer_connection::data_handler, this, std::placeholders::_1,
                                       std::placeholders::_2));
    }

    tcp_peer_connection::tcp_peer_connection(tcp_peer_connection &&other) noexcept
            : peer_connection(std::move(other)),
              m_socket(std::move(other.m_socket)),
              m_socketEvent(other.m_socketEvent),
              m_buffer(std::move(other.m_buffer)) {
    }

    tcp_peer_connection::~tcp_peer_connection() noexcept {
        DEBUG_LOG("TCP Peer connection destroyed");
        m_queue->unregisterEvent(m_socketEvent);
    }

    MessageCodec::MessageDecodeStatus tcp_peer_connection::process_buffer(const MessageBuffer::ByteStorageType &buffer) noexcept {
        const char *subject_ptr = nullptr;
        size_t subject_length = 0;
        uint8_t flags = 0;
        size_t msg_length = 0;

        buffer.mark();
        auto status = MessageCodec::addressing_details(buffer, &subject_ptr, subject_length, flags, msg_length);
        if (status == MessageCodec::CompleteMessage) {
            buffer.resetRead();
            if (subject_ptr != nullptr && subject_length > 0) {
                DEBUG_LOG("Received message [" << subject_ptr << "] of length " << msg_length << ": read " << buffer.bytesRead() << " of a total " << m_buffer.length());
                subject<> subject(subject_ptr);
                const MessageBuffer::ByteStorageType &msgData = buffer.segment(msg_length);
                if (tf::unlikely(subject.is_admin())) {
                    Message message;
                    if (MessageCodec::decode(&message, msgData)) {
                        this->handle_admin_message(subject, message);
                    } else {
                        ERROR_LOG("Failed to decode message: " << msgData);
                        status = MessageCodec::CorruptMessage;
                    }
                } else {
                    // otherwise pass it to our handler
                    m_messageHandler(this, subject, message_wrapper(msgData));
                }
                buffer.mark();
            } else {
                ERROR_LOG("Message has null or zero length subject");
                status = MessageCodec::CorruptMessage;
            }
        } else {
            buffer.resetRead();
        }

        return status;
    }

    void tcp_peer_connection::data_handler(DataEvent *event, const EventType eventType) noexcept {

        static const size_t MTU_SIZE = 1500;

        ssize_t size = 0;
        bool complete = false;
        while (!complete) {
            DEBUG_LOG(this << " ------------");
            Socket::ReadResult result = m_socket->read(reinterpret_cast<const char *>(m_buffer.allocate(MTU_SIZE)), MTU_SIZE, size);
            DEBUG_LOG("Read " << size << " bytes [total buffer size: " << m_buffer.length() << "]");
            m_buffer.erase_back(MTU_SIZE - size);
            DEBUG_LOG("Removed trailing " << MTU_SIZE - size << " bytes [total buffer size: " << m_buffer.length() << "]");
            if (result == Socket::MoreData) {
                const MessageBuffer::ByteStorageType &storage = m_buffer.byteStorage();

                MessageCodec::MessageDecodeStatus status;
                while ((status = this->process_buffer(storage)) == MessageCodec::CompleteMessage);

                switch (status) {
                    case MessageCodec::CompleteMessage:
                        break;
                    case MessageCodec::IncompleteMessage:
                        DEBUG_LOG("Removing " << storage.bytesRead() << " from front [" << m_buffer.length() << "] ");
                        m_buffer.erase_front(storage.bytesRead());
                        DEBUG_LOG("Removed " << storage.bytesRead() << " from front [" << m_buffer.length() << "] ");
                        break;
                    case MessageCodec::CorruptMessage:
                        m_socket->disconnect();
                        m_disconnectionHandler(this);
                        complete = true;
                        break;
                }

            } else if (result == Socket::NoData) {
                complete = true;
            } else if (result == Socket::Closed) {
                DEBUG_LOG("Client Socket closed");
                m_disconnectionHandler(this);
                complete = true;
            }
        }
    }

    bool tcp_peer_connection::sendBuffer(const message_wrapper &wrapper) noexcept {
        auto &buffer = wrapper.getEncodedBuffer();
        DEBUG_LOG("Sending " << buffer.length() << " bytes to client");
        const byte *data;
        const size_t len = buffer.bytes(&data);
        return m_socket->send(reinterpret_cast<const char *>(data), len);
    }
}