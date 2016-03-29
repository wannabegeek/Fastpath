//
// Created by Tom Fewster on 27/03/2016.
//

#ifndef TFDCF_SHMTRANSPORT_H
#define TFDCF_SHMTRANSPORT_H

#include <status.h>
#include <messages/Message.h>
#include "URL.h"
#include "Transport.h"

namespace DCF {
    class SHMTransport : public Transport {
    private:
        virtual std::unique_ptr<TransportIOEvent> createReceiverEvent(const std::function<void(const Transport *, MessageType &)> &messageCallback) override;

    public:
        // This should have the format dcf://localhost:1234
        SHMTransport(const char *url, const char *description);
        SHMTransport(const url &url, const char *description);

        SHMTransport(const SHMTransport &) = delete;
        SHMTransport &operator=(const SHMTransport &) = delete;

        ~SHMTransport();

        status sendMessage(const Message &msg) override;
        status sendMessageWithResponse(const Message &request, Message &reply, std::chrono::duration<std::chrono::milliseconds> &timeout) override;
        status sendReply(const Message &reply, const Message &request) override;
        const bool valid() const noexcept override;

    };
}

#endif //TFDCF_SHMTRANSPORT_H
