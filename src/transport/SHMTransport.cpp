//
// Created by Tom Fewster on 27/03/2016.
//

#include "SHMTransport.h"

namespace DCF {
    SHMTransport::SHMTransport(const char *url_ptr, const char *description) : SHMTransport(url(url_ptr), description) {
    }

    SHMTransport::SHMTransport(const url &url, const char *description) : Transport(description) {

    }

    SHMTransport::~SHMTransport() {
    }

    status SHMTransport::sendMessage(const Message &msg) {
        return OK;
    }

    status SHMTransport::sendMessageWithResponse(const Message &request, Message &reply, std::chrono::duration<std::chrono::milliseconds> &timeout) {
        return OK;
    }

    status SHMTransport::sendReply(const Message &reply, const Message &request) {
        return OK;
    }

    const bool SHMTransport::valid() const noexcept {
        return false;
    }

    std::unique_ptr<TransportIOEvent> SHMTransport::createReceiverEvent(const std::function<void(const Transport *, Message *)> &messageCallback) {
        return nullptr;
    }
}


