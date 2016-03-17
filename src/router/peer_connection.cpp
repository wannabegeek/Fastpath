//
// Created by Tom Fewster on 11/03/2016.
//

#include <event/Queue.h>
#include <messages/Message.h>
#include "peer_connection.h"
#include "subject.h"
#include "transport/Socket.h"

namespace fp {
    peer_connection::peer_connection(DCF::Queue *queue, std::unique_ptr<DCF::Socket> socket, const std::function<void(peer_connection *, const subject<> &, DCF::ByteStorage &)> messageHandler, const std::function<void(peer_connection *)> &disconnectionHandler)
            : m_queue(queue), m_socket(std::move(socket)), m_buffer(4500), m_messageHandler(messageHandler), m_disconnectionHandler(disconnectionHandler) {

        m_socketEvent.create(queue, m_socket->getSocket(), DCF::EventType::READ,
                             std::bind(&peer_connection::data_handler, this, std::placeholders::_1,
                                       std::placeholders::_2));
    }

    peer_connection::peer_connection(peer_connection &&other)
            : m_subscriptions(std::move(other.m_subscriptions)),
              m_queue(other.m_queue),
              m_socket(std::move(other.m_socket)),
              m_buffer(std::move(other.m_buffer)),
              m_disconnectionHandler(other.m_disconnectionHandler) {
    }

    peer_connection::~peer_connection() {
        DEBUG_LOG("Peer connection destroyed");
    }

    void peer_connection::add_subscription(const char *subject) {
        m_subscriptions.emplace_back(subject);
    }

    void peer_connection::remove_subscription(const char *subject) {
        auto it = std::find_if(m_subscriptions.begin(), m_subscriptions.end(), [&](const subscription<> &s) {
            return s == subject;
        });
        if (it != m_subscriptions.end()) {
            m_subscriptions.erase(it);
        }
    }

    bool peer_connection::is_interested(const subject<> &subject) const {
        auto it = std::find_if(m_subscriptions.begin(), m_subscriptions.end(), [&](const subscription<> &s) {
            return s.matches(subject);
        });

        return (it != m_subscriptions.end());
    }

    void peer_connection::data_handler(DCF::IOEvent *event, const DCF::EventType eventType) {
        DEBUG_LOG("Received data from client");

        static const size_t MTU_SIZE = 1500;

        ssize_t size = 0;
        while (true) {
            DCF::Socket::ReadResult result = m_socket->read(reinterpret_cast<const char *>(m_buffer.allocate(MTU_SIZE)), MTU_SIZE, size);
            m_buffer.erase_back(MTU_SIZE - size);
            if (result == DCF::Socket::MoreData) {
                const DCF::ByteStorage &storage = m_buffer.byteStorage();
                const char *subject_ptr = nullptr;
                size_t subject_length = 0;
                uint8_t flags = 0;
                size_t msg_length = 0;

                try {
                    if (DCF::Message::addressing_details(storage, &subject_ptr, subject_length, flags, msg_length)) {
                        INFO_LOG("Received message [" << subject_ptr << "] of length " << msg_length);
                        const byte *data = nullptr;
                        m_buffer.bytes(&data);
                        subject<> subject(subject_ptr);
                        DCF::ByteStorage msgData(data, msg_length, true);
                        if (tf::unlikely(subject.is_admin())) {
                            DEBUG_LOG("Received admin message");
                        } else {
                            // other wise pass it to our handler
                            m_messageHandler(this, subject, msgData);
                        }
                        m_buffer.erase_front(msg_length);
                    } else {
                        storage.resetRead();
                        break;
                    }
                } catch (fp::exception &e) {
                    ERROR_LOG(e);
                    m_socket->disconnect();
                    m_disconnectionHandler(this);
                }

            } else if (result == DCF::Socket::NoData) {
                break;
            } else if (result == DCF::Socket::Closed) {
                DEBUG_LOG("Client Socket closed");
                m_disconnectionHandler(this);
                break;
            }
        }
    }

    bool peer_connection::sendBuffer(const DCF::ByteStorage &buffer) {
        const byte *data;
        const size_t len = buffer.bytes(&data);
        return m_socket->send(reinterpret_cast<const char *>(data), len);
    }
}