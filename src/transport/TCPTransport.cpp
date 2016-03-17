//
// Created by Tom Fewster on 04/03/2016.
//

#include <future>
#include <chrono>
#include "utils/logger.h"

#include "TCPTransport.h"
#include "SocketClient.h"
#include "messages/Message.h"

namespace DCF {

    class BackoffStrategy {
    private:
        static constexpr std::chrono::milliseconds min_retry_interval() { return std::chrono::milliseconds(1); }
        static constexpr std::chrono::milliseconds max_retry_interval() { return std::chrono::seconds(5); }

        std::chrono::milliseconds m_current;
    public:
        BackoffStrategy() : m_current(min_retry_interval()) {}

        void backoff() {
            std::this_thread::sleep_for(m_current);
            m_current = std::min(m_current * 2, max_retry_interval());
        }
    };


    TCPTransport::TCPTransport(const char *url_ptr, const char *description) : TCPTransport(url(url_ptr), description) {
    }

    TCPTransport::TCPTransport(const url &url, const char *description) : Transport(description), m_url(url), m_shouldDisconnect(false), m_sendBuffer(1024) {
        INFO_LOG("Connecting to: " << m_url);
        m_peer = std::make_unique<SocketClient>(m_url.host(), m_url.port());

        if (!m_peer->connect()) {
            m_connectionAttemptInProgress = std::async(std::launch::async, &TCPTransport::__connect, this);
        }
    }

    TCPTransport::~TCPTransport() {
        this->__disconnect();
    }

    bool TCPTransport::__connect() {
        BackoffStrategy strategy;
        while (!m_peer->isConnected() && m_shouldDisconnect == false) {
            if (!m_peer->connect()) {
                INFO_LOG("Failed to connect trying again");
                strategy.backoff();
            }
        }

        DEBUG_LOG("Either connected or given up");
        return true;
    }

    bool TCPTransport::__disconnect() {
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

    status TCPTransport::sendMessage(const Message &msg) {
        if (m_peer->isConnected()) {
            msg.encode(m_sendBuffer);
            const byte *data = nullptr;
            size_t len = m_sendBuffer.bytes(&data);
            return m_peer->send(reinterpret_cast<const char *>(data), len) ? OK : CANNOT_SEND;
        }
        return CANNOT_SEND;
    }

    status TCPTransport::sendMessageWithResponse(const Message &request, Message &reply,
                                                         std::chrono::duration<std::chrono::milliseconds> &timeout) {
        return this->sendMessage(request);
    }

    status TCPTransport::sendReply(const Message &reply, const Message &request) {
        return this->sendMessage(reply);
    }

    const bool TCPTransport::valid() const noexcept {
        return m_peer->isConnected();
    }
}