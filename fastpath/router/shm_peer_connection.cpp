/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 26/03/2016

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

#include "shm_peer_connection.h"

#include "fastpath/transport/sm/InterprocessNotifierServer.h"
#include "fastpath/transport/socket/UnixSocket.h"
#include "fastpath/SharedMemoryBuffer.h"
#include "fastpath/messages/MutableMessage.h"
#include "fastpath/messages/Message.h"
#include "fastpath/messages/MessageCodec.h"
#include "fastpath/event/Queue.h"
#include "fastpath/router/message_wrapper.h"

namespace fp {
    shm_peer_connection::shm_peer_connection(Queue *queue, InterprocessNotifierServer::notifier_type &&notifier, std::unique_ptr<UnixSocket> &&socket, int process_id, SharedMemoryManager *manager)
            : peer_connection(queue),
              m_notifier(std::forward<InterprocessNotifierServer::notifier_type>(notifier)),
              m_socket(std::forward<std::unique_ptr<UnixSocket>>(socket)),
              m_process_id(process_id) {

        char queueName[32];
        ::sprintf(queueName, "ClientQueue_%i", process_id);
        m_clientQueue = std::make_unique<SharedMemoryBuffer>(queueName, *manager);
        ::sprintf(queueName, "ServerQueue_%i", process_id);
        m_serverQueue = std::make_unique<SharedMemoryBuffer>(queueName, *manager);

        this->socketEvent();
        this->notificationEvent();
    }

    shm_peer_connection::shm_peer_connection(shm_peer_connection &&other) noexcept
            : peer_connection(std::move(other)),
              m_notifier(std::move(other.m_notifier)),
              m_clientQueue(std::move(other.m_clientQueue)),
              m_serverQueue(std::move(other.m_serverQueue)),
              m_socket(std::move(other.m_socket)),
              m_process_id(std::move(other.m_process_id)),
              m_socketEvent(std::move(other.m_socketEvent)),
              m_notifierEvent(std::move(other.m_notifierEvent)) {
    }

    shm_peer_connection::~shm_peer_connection() noexcept {
        DEBUG_LOG("SHM Peer connection destroyed");
        m_queue->unregisterEvent(m_socketEvent);
        m_queue->unregisterEvent(m_notifierEvent);
    }

    void shm_peer_connection::notificationHandler(DataEvent *event, const fp::EventType type) noexcept {
        auto &notifier = std::get<0>(m_notifier);
        if (tf::unlikely(!notifier->reset())) {
            ERROR_LOG("BOOM - Client disconnected: " << m_process_id);
            if (m_disconnectionHandler) {
                m_disconnectionHandler(this);
            }
        } else {
            DEBUG_LOG("We have been notified of a message from: " << m_process_id);
            m_serverQueue->retrieve([&](auto &ptr, SharedMemoryManager::shared_ptr_type &shared_ptr) {

                const char *subject_ptr = nullptr;
                size_t subject_length = 0;
                uint8_t flags = 0;
                size_t msg_length = 0;

                ptr.mark();
                auto status = MessageCodec::addressing_details(ptr, &subject_ptr, subject_length, flags, msg_length);
                if (status == MessageCodec::CompleteMessage) {
                    DEBUG_LOG("Subject is: " << std::string(subject_ptr, subject_length));
                } else {
                    ERROR_LOG("Somehow SHM has given us an incomplete message");
                    return;
                }
                ptr.resetRead();

                subject<> subject(subject_ptr);
                if (tf::unlikely(subject.is_admin())) {
                    Message message;
                    if (MessageCodec::decode(&message, ptr)) {
                        DEBUG_LOG("Received admin message: " << message);
                        this->handle_admin_message(subject, message);
                    } else {
                        ERROR_LOG("Failed to decode message: " << ptr);
                        status = MessageCodec::CorruptMessage;
                    }
                } else {
                    message_wrapper mw(ptr, shared_ptr);
                    m_messageHandler(this, subject, mw);
                }
            });
        }
    }

    void shm_peer_connection::socketEvent() {
        if (!m_socketEvent) {
            m_socketEvent = m_queue->registerEvent(m_socket->getSocket(), EventType::READ, [&](DataEvent *event, const EventType type) {
                const char buffer[256] = {0};
                ssize_t len = 0;
                while (true) {
                    Socket::ReadResult r = m_socket->read(buffer, 255, len);
                    switch (r) {
                        case fp::Socket::MoreData:
                            break;
                        case fp::Socket::NoData:
                            break;
                        case fp::Socket::Closed: {
                            ERROR_LOG("BOOM - Client disconnected: " << m_process_id);
                            if (m_disconnectionHandler) {
                                m_disconnectionHandler(this);
                            }

                            return;
                        }
                    }
                }
            });
        }
    }

    void shm_peer_connection::notificationEvent() {
        if (!m_notifierEvent) {
            int fd = std::get<0>(m_notifier)->read_handle();
            DEBUG_LOG("Adding callback for " << fd);
            m_notifierEvent = m_queue->registerEvent(fd, EventType::READ, std::bind(&shm_peer_connection::notificationHandler, this, std::placeholders::_1, std::placeholders::_2));
        }
    }

    bool shm_peer_connection::sendBuffer(const message_wrapper &wrapper) noexcept {
        auto &send_buffer = wrapper.getSharedPtrBuffer();
        DEBUG_LOG("Sending to: " << m_process_id);

        m_clientQueue->notify(send_buffer);
        std::get<1>(m_notifier)->notify();

        return true;
    }

}
