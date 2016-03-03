//
// Created by Tom Fewster on 29/02/2016.
//

#ifndef TFDCF_TRANSPORT_H
#define TFDCF_TRANSPORT_H

#include <messages/StorageTypes.h>
#include <chrono>
#include <string>
#include <status.h>

namespace DCF {
    class Transport {
        const std::string m_description;
    public:
        virtual ~Transport(const char *description) : m_description(description) {};

        virtual status sendMessage(const MessageType &msg) = 0;
        virtual status sendMessageWithResponse(const MessageType &request, MessageType &reply, std::chrono::duration<std::chrono::milliseconds> &timeout) = 0;

        virtual status sendReply(const MessageType &reply, const Message &request) = 0;

        virtual const bool valid() const noexcept = 0;

        const char *description() const noexcept {
            return m_description.c_str();
        }
    };
}

#endif //TFDCF_TRANSPORT_H
