//
// Created by Tom Fewster on 04/03/2016.
//

#include <future>
#include <chrono>

#include "TCPTransport.h"
#include "SocketClient.h"

namespace DCF {
    TCPTransport::TCPTransport(const char *url, const char *description) : Transport(description) {
        const std::string host = "localhost";
        const std::string port = "6969";
        m_peer = std::make_unique<SocketClient>(host, port);
        std::future<bool> result = std::async(std::launch::async, __connect);
    }

    bool TCPTransport::__connect() {
        std::chrono::milliseconds backoff = std::chrono::milliseconds(1);
        while (!m_peer->isConnected()) {
            if (!m_peer->connect()) {
                std::this_thread::sleep_for(backoff);
            } else {
                backoff = std::chrono::milliseconds(1);
            }
        }
    }

    status TCPTransport::sendMessage(const MessageType &msg) {
        return CANNOT_SEND;
    }

    status TCPTransport::sendMessageWithResponse(const MessageType &request, MessageType &reply,
                                                         std::chrono::duration<std::chrono::milliseconds> &timeout) {
        return CANNOT_SEND;
    }

    status TCPTransport::sendReply(const MessageType &reply, const Message &request) {
        return CANNOT_SEND;
    }

    const bool TCPTransport::valid() const noexcept {
        return false;
    }
}