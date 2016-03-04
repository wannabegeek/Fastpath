//
// Created by Tom Fewster on 04/03/2016.
//

#ifndef TFDCF_TCPTRANSPORT_H
#define TFDCF_TCPTRANSPORT_H

#include "Transport.h"

namespace DCF {
    class SocketClient;

    class TCPTransport : public Transport {
    private:
        std::unique_ptr<SocketClient> m_peer;
        bool __connect();
    public:
        // This should have the format dcf://localhost:1234
        TCPTransport(const char *url, const char *description);

        status sendMessage(const MessageType &msg) override;
        status sendMessageWithResponse(const MessageType &request, MessageType &reply, std::chrono::duration<std::chrono::milliseconds> &timeout) override;
        status sendReply(const MessageType &reply, const Message &request) override;
        const bool valid() const noexcept override;

    };
}

#endif //TFDCF_TCPTRANSPORT_H
