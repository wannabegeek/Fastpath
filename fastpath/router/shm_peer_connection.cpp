//
// Created by Tom Fewster on 23/05/2016.
//

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
    shm_peer_connection::shm_peer_connection(Queue *queue, InterprocessNotifierServer::notifier_type &&notifier, std::unique_ptr<UnixSocket> &&socket, int process_id, SharedMemoryManager *manager, const std::function<void(const std::function<void(const subject<> &, const message_wrapper &)>)> &message_reader)
            : peer_connection(queue),
              m_notifier(std::forward<InterprocessNotifierServer::notifier_type>(notifier)),
              m_socket(std::forward<std::unique_ptr<UnixSocket>>(socket)),
              m_process_id(process_id),
              m_message_reader(message_reader) {

        char queueName[32];
        ::sprintf(queueName, "ClientQueue_%i", process_id);
        m_clientQueue = std::make_unique<SharedMemoryBuffer>(queueName, *manager);

        this->socketEvent();
        this->notificationEvent();
    }

    shm_peer_connection::shm_peer_connection(shm_peer_connection &&other) noexcept
            : peer_connection(std::move(other)),
              m_notifier(std::move(other.m_notifier)),
              m_clientQueue(std::move(other.m_clientQueue)),
              m_socket(std::move(other.m_socket)),
              m_process_id(std::move(other.m_process_id)),
              m_socketEvent(std::move(other.m_socketEvent)),
              m_notifierEvent(std::move(other.m_notifierEvent)),
              m_message_reader(std::move(other.m_message_reader)) {

    }

    shm_peer_connection::~shm_peer_connection() noexcept {
        DEBUG_LOG("SHM Peer connection destroyed");
        m_queue->unregisterEvent(m_socketEvent);
        m_queue->unregisterEvent(m_notifierEvent);
    }

    void shm_peer_connection::notificationHandler(DataEvent *event, const fp::EventType type) noexcept {
        auto &notifier = std::get<0>(m_notifier);
        if (!notifier->reset()) {
            ERROR_LOG("BOOM - Client disconnected: " << m_process_id);
            if (m_disconnectionHandler) {
                m_disconnectionHandler(this);
            }
        } else {
            INFO_LOG("We have been notified of a message: " << m_process_id);
            // TODO - this is horrible & shouldn't be creating this on every message
            m_message_reader(std::bind(m_messageHandler, this, std::placeholders::_1, std::placeholders::_2));
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
        INFO_LOG("Sending to: " << m_process_id);

        m_clientQueue->notify(send_buffer);
        std::get<1>(m_notifier)->notify();

        return true;
    }

}