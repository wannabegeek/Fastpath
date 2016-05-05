/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef FASTPATH_TRANSPORT_H
#define FASTPATH_TRANSPORT_H

#include <chrono>
#include <string>
#include <functional>

#include "fastpath/status.h"
#include "fastpath/messages/StorageTypes.h"
#include "fastpath/utils/tfspinlock.h"
#include "fastpath/utils/tfpool.h"
#include "fastpath/messages/Message.h"
#include "fastpath/transport/TransportIOEvent.h"

namespace fp {
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

        typedef tf::pool<Message, tf::spinlock> MessagePoolType;
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
        virtual ~Transport() noexcept = default;

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

#endif //FASTPATH_TRANSPORT_H
