//
// Created by Tom Fewster on 29/02/2016.
//

#ifndef TFDCF_TRANSPORT_H
#define TFDCF_TRANSPORT_H

#include <messages/StorageTypes.h>
#include <transport/TransportIOEvent.h>
#include <utils/tfpool.h>
#include <messages/Message.h>
#include <chrono>
#include <string>
#include <functional>
#include <status.h>

namespace DCF {
    class EventManager;
    class TransportIOEvent;

    class Transport {
        friend class MessageListener;
    public:
        typedef enum {
            CONNECTED,
            DISCONNECTED,
            CORRUPT_MESSAGE,
            SLOW_CONSUMER
        } notification_type;

        typedef tf::pool<Message> MessagePoolType;
        using MessageType = MessagePoolType::shared_ptr_type;

    private:
        const std::string m_description;
        const EventManager *m_eventManager = nullptr;
    protected:
        MessagePoolType m_msg_pool;

        std::function<void(notification_type type, const char *reason)> m_notificationHandler;

        virtual std::unique_ptr<TransportIOEvent> createReceiverEvent(const std::function<void(const Transport *, MessageType &)> &messageCallback) = 0;

    public:
        Transport(const char *description) : m_description(description), m_msg_pool(10000) {};
        virtual ~Transport() noexcept {}

        status sendMessage(const MessageType &msg) noexcept {
            return sendMessage(*(msg.get()));
        };

        virtual status sendMessage(const Message &msg) noexcept = 0;
        virtual status sendMessageWithResponse(const Message &request, Message &reply, std::chrono::duration<std::chrono::milliseconds> &timeout) noexcept = 0;

        virtual status sendReply(const Message &reply, const Message &request) noexcept = 0;

        virtual const bool valid() const noexcept = 0;

        const char *description() const noexcept {
            return m_description.c_str();
        }

        void setNotificationHandler(std::function<void(notification_type type, const char *reason)> handler) noexcept {
            m_notificationHandler = handler;
        }
    };
}

#endif //TFDCF_TRANSPORT_H
