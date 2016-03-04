//
// Created by Tom Fewster on 04/03/2016.
//

#include <future>
#include <chrono>

#include "TCPTransport.h"
#include "SocketClient.h"

namespace DCF {

    class BackoffStrategy {
    private:
        static constexpr std::chrono::milliseconds min_retry_interval() { return std::chrono::milliseconds(1); }
        static constexpr std::chrono::milliseconds max_retry_interval() { return std::chrono::seconds(15); }

        std::chrono::milliseconds m_current;
    public:
        BackoffStrategy() : m_current(min_retry_interval()) {}

        void backoff() {
            std::this_thread::sleep_for(m_current);
            m_current = std::min(m_current * 2, max_retry_interval());
        }
    };


    TCPTransport::TCPTransport(const char *url, const char *description) : Transport(description) {
        const std::string host = "localhost";
        const std::string port = "6969";
        m_peer = std::make_unique<SocketClient>(host, port);
        auto result = std::async(std::launch::async, &TCPTransport::__connect, this);
    }

    bool TCPTransport::__connect() {
        BackoffStrategy strategy;
        while (!m_peer->isConnected()) {
            if (!m_peer->connect()) {
                strategy.backoff();
            }
        }

        return true;
    }

    status TCPTransport::sendMessage(const MessageType &msg) {
        if (m_peer->isConnected()) {

            return OK;
        }
        return CANNOT_SEND;
    }

    status TCPTransport::sendMessageWithResponse(const MessageType &request, MessageType &reply,
                                                         std::chrono::duration<std::chrono::milliseconds> &timeout) {
        return this->sendMessage(request);
    }

    status TCPTransport::sendReply(const MessageType &reply, const Message &request) {
        return this->sendMessage(reply);
    }

    const bool TCPTransport::valid() const noexcept {
        return false;
    }
}