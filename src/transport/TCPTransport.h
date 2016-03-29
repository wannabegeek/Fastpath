//
// Created by Tom Fewster on 04/03/2016.
//

#ifndef TFDCF_TCPTRANSPORT_H
#define TFDCF_TCPTRANSPORT_H

#include "Transport.h"
#include "SocketClient.h"
#include "URL.h"
#include <future>
#include <MessageBuffer.h>

namespace DCF {

    class Event;
    class TransportIOEvent;

    class TCPTransport : public Transport {
    private:
        virtual std::unique_ptr<TransportIOEvent> createReceiverEvent(const std::function<void(const Transport *, MessageType &)> &messageCallback) override;
    protected:
        std::unique_ptr<SocketClient> m_peer;
        const url m_url;

        std::atomic<bool> m_shouldDisconnect;
        std::future<bool> m_connectionAttemptInProgress;

        MessageBuffer m_sendBuffer;
        MessageBuffer m_readBuffer;

        bool __connect();
        bool __disconnect();
    public:
        // This should have the format dcf://localhost:1234
        TCPTransport(const char *url, const char *description);
        TCPTransport(const url &url, const char *description);

        TCPTransport(const TCPTransport &) = delete;
        TCPTransport &operator=(const TCPTransport &) = delete;

        ~TCPTransport();

        status sendMessage(const Message &msg) override;
        status sendMessageWithResponse(const Message &request, Message &reply, std::chrono::duration<std::chrono::milliseconds> &timeout) override;
        status sendReply(const Message &reply, const Message &request) override;
        const bool valid() const noexcept override;

    };
}

#endif //TFDCF_TCPTRANSPORT_H
