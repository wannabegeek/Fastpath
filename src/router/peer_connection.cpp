//
// Created by Tom Fewster on 11/03/2016.
//

#include <event/Queue.h>
#include <messages/Message.h>
#include "peer_connection.h"
#include "subject.h"
#include "transport/Socket.h"

namespace fp {
    peer_connection::peer_connection(DCF::Queue *queue, std::unique_ptr<DCF::Socket> socket, const std::function<void(peer_connection *, const subject<> &, const DCF::MessageBuffer::ByteStorageType &)> messageHandler, const std::function<void(peer_connection *)> &disconnectionHandler)
            : m_queue(queue), m_socket(std::move(socket)), m_buffer(4500), m_messageHandler(messageHandler), m_disconnectionHandler(disconnectionHandler) {

        m_socketEvent = queue->registerEvent(m_socket->getSocket(), DCF::EventType::READ,
                             std::bind(&peer_connection::data_handler, this, std::placeholders::_1,
                                       std::placeholders::_2));
    }

    peer_connection::peer_connection(peer_connection &&other) noexcept
            : m_subscriptions(std::move(other.m_subscriptions)),
              m_queue(other.m_queue),
              m_socket(std::move(other.m_socket)),
              m_socketEvent(other.m_socketEvent),
              m_buffer(std::move(other.m_buffer)),
              m_disconnectionHandler(other.m_disconnectionHandler) {
    }

    peer_connection::~peer_connection() noexcept {
        DEBUG_LOG("Peer connection destroyed");
        m_queue->unregisterEvent(m_socketEvent);
    }

    void peer_connection::add_subscription(const char *subject) noexcept {
        m_subscriptions.emplace_back(subject);
    }

    void peer_connection::remove_subscription(const char *subject) noexcept {
        auto it = std::find_if(m_subscriptions.begin(), m_subscriptions.end(), [&](const subscription<> &s) noexcept {
            return s == subject;
        });
        if (it != m_subscriptions.end()) {
            m_subscriptions.erase(it);
        }
    }

    bool peer_connection::is_interested(const subject<> &subject) const noexcept {
        auto it = std::find_if(m_subscriptions.begin(), m_subscriptions.end(), [&](const subscription<> &s) noexcept {
            return s.matches(subject);
        });

        return (it != m_subscriptions.end());
    }

    void peer_connection::handle_admin_message(const subject<> subject, DCF::Message &message) noexcept {
        DEBUG_LOG("Received admin msg: " << message);

        if (subject == RegisterObserver()) {
            const char *data = nullptr;
            size_t len = 0;
            if (!message.getDataField("subject", &data, len) || len == 0) {
                ERROR_LOG("Received invalid message - subscription without a subject to subscribe to");
                return;
            }
            this->add_subscription(data);
        } else if (subject == RegisterObserver()) {
            const char *data = nullptr;
            size_t len = 0;
            if (!message.getDataField("subject", &data, len) && len > 0) {
                ERROR_LOG("Received invalid message - subscription without a subject to subscribe to");
                return;
            }
            this->remove_subscription(data);
        }
    }

    DCF::MessageDecodeStatus peer_connection::process_buffer(const DCF::MessageBuffer::ByteStorageType &buffer) noexcept {
        const char *subject_ptr = nullptr;
        size_t subject_length = 0;
        uint8_t flags = 0;
        size_t msg_length = 0;

        buffer.mark();
        auto status = DCF::Message::addressing_details(buffer, &subject_ptr, subject_length, flags, msg_length);
        if (status == DCF::CompleteMessage) {
            buffer.resetRead();
            if (subject_ptr != nullptr && subject_length > 0) {
                DEBUG_LOG(
                        "Received message [" << subject_ptr << "] of length " << msg_length << ": read " <<
                        buffer.bytesRead() << " of a total " << m_buffer.length());
                subject<> subject(subject_ptr);
                const DCF::MessageBuffer::ByteStorageType &msgData = buffer.segment(msg_length);
                if (tf::unlikely(subject.is_admin())) {
                    DCF::Message message;
                    if (message.decode(msgData)) {
                        this->handle_admin_message(subject, message);
                    } else {
                        ERROR_LOG("Failed to decode message: " << msgData);
                        status = DCF::CorruptMessage;
                    }
                } else {
                    // otherwise pass it to our handler
                    m_messageHandler(this, subject, msgData);
                }
                buffer.mark();
            } else {
                ERROR_LOG("Message has null or zero length subject");
                status = DCF::CorruptMessage;
            }
        } else {
            buffer.resetRead();
        }

        return status;
    }

    void peer_connection::data_handler(DCF::DataEvent *event, const DCF::EventType eventType) noexcept {

        static const size_t MTU_SIZE = 1500;

        ssize_t size = 0;
        bool complete = false;
        while (!complete) {
            DEBUG_LOG(this << " ------------");
            DCF::Socket::ReadResult result = m_socket->read(reinterpret_cast<const char *>(m_buffer.allocate(MTU_SIZE)), MTU_SIZE, size);
            DEBUG_LOG("Read " << size << " bytes [total buffer size: " << m_buffer.length() << "]");
            m_buffer.erase_back(MTU_SIZE - size);
            DEBUG_LOG("Removed trailing " << MTU_SIZE - size << " bytes [total buffer size: " << m_buffer.length() << "]");
            if (result == DCF::Socket::MoreData) {
                const DCF::MessageBuffer::ByteStorageType &storage = m_buffer.byteStorage();

                DCF::MessageDecodeStatus status;
                while ((status = this->process_buffer(storage)) == DCF::CompleteMessage);

                switch (status) {
                    case DCF::CompleteMessage:
                        break;
                    case DCF::IncompleteMessage:
                        DEBUG_LOG("Removing " << storage.bytesRead() << " from front [" << m_buffer.length() << "] ");
                        m_buffer.erase_front(storage.bytesRead());
                        DEBUG_LOG("Removed " << storage.bytesRead() << " from front [" << m_buffer.length() << "] ");
                        complete = true;
                        break;
                    case DCF::CorruptMessage:
                        m_socket->disconnect();
                        m_disconnectionHandler(this);
                        complete = true;
                        break;
                }

            } else if (result == DCF::Socket::NoData) {
                complete = true;
            } else if (result == DCF::Socket::Closed) {
                DEBUG_LOG("Client Socket closed");
                m_disconnectionHandler(this);
                complete = true;
            }
        }
    }

    bool peer_connection::sendBuffer(const DCF::MessageBuffer::ByteStorageType &buffer) noexcept {
        DEBUG_LOG("Sending " << buffer.length() << " bytes to client");
        const byte *data;
        const size_t len = buffer.bytes(&data);
        return m_socket->send(reinterpret_cast<const char *>(data), len);
    }
}