//
// Created by Tom Fewster on 29/02/2016.
//

#ifndef TFDCF_TRANSPORT_H
#define TFDCF_TRANSPORT_H

#include <messages/StorageTypes.h>
#include <chrono>
#include <string>
#include <functional>
#include <status.h>

namespace DCF {
    class EventManager;

    class Transport {
        friend class MessageListener;
    public:
        typedef enum {
            CONNECTED,
            DISCONNECTED,
            CORRUPT_MESSAGE,
            SLOW_CONSUMER
        } notification_type;
    private:
        const std::string m_description;
        const EventManager *m_eventManager = nullptr;
    protected:
        std::function<void(notification_type type, const char *reason)> m_notificationHandler;
    public:
        Transport(const char *description) : m_description(description) {};
        virtual ~Transport() {}

        virtual status sendMessage(const Message &msg) = 0;
        virtual status sendMessageWithResponse(const Message &request, Message &reply, std::chrono::duration<std::chrono::milliseconds> &timeout) = 0;

        virtual status sendReply(const Message &reply, const Message &request) = 0;

        virtual const bool valid() const noexcept = 0;

        const char *description() const noexcept {
            return m_description.c_str();
        }

        void setNotificationHandler(std::function<void(notification_type type, const char *reason)> handler) {
            m_notificationHandler = handler;
        }
    };
}

#endif //TFDCF_TRANSPORT_H
