//
// Created by Tom Fewster on 29/02/2016.
//

#ifndef TFDCF_TRANSPORT_H
#define TFDCF_TRANSPORT_H

#include <messages/StorageTypes.h>
#include <chrono>

namespace DCF {
    class Transport {
    public:
        virtual ~Transport() {};

        virtual void sendMessage(const MessageType &msg) = 0;
        virtual void sendMessageWithResponse(const MessageType &msg, MessageType &msg, std::chrono::duration<std::chrono::milliseconds> &timeout) = 0;
    };
}

#endif //TFDCF_TRANSPORT_H
