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

#ifndef FASTPATH_PEER_CONNECTION_H
#define FASTPATH_PEER_CONNECTION_H

#include <string>
#include <vector>
#include <memory>

#include "fastpath/messages/MessageCodec.h"
#include "fastpath/event/IOEvent.h"
#include "fastpath/messages/Message.h"
#include "fastpath/MessageBuffer.h"
#include "fastpath/messages/subscription.h"

namespace fp {
    class Socket;
    class Queue;
}

namespace fp {
    class peer_connection {
    private:
        // TODO: these need to be calculated up-front
        static subject<> RegisterObserver() { return subject<>("_FP.REGISTER.OBSERVER"); }
        static subject<> UnregisterObserver() { return subject<>("_FP.UNREGISTER.OBSERVER"); }

        std::vector<subscription<>> m_subscriptions;
        Queue *m_queue;
        std::unique_ptr<Socket> m_socket;
        DataEvent *m_socketEvent;

        MessageBuffer m_buffer;
        const std::function<void(peer_connection *, const subject<> &, const MessageBuffer::ByteStorageType &)> m_messageHandler;
        const std::function<void(peer_connection *)> m_disconnectionHandler;

        void data_handler(DataEvent *event, const EventType eventType) noexcept;
        MessageCodec::MessageDecodeStatus process_buffer(const MessageBuffer::ByteStorageType &buffer) noexcept;

        void handle_admin_message(const subject<> subject, Message &message) noexcept;
    public:
        peer_connection(Queue *queue, std::unique_ptr<Socket> socket, const std::function<void(peer_connection *, const subject<> &, const MessageBuffer::ByteStorageType &)> messageHandler, const std::function<void(peer_connection *)> &disconnectionHandler);
        peer_connection(peer_connection &&other) noexcept;
        ~peer_connection() noexcept;

        void add_subscription(const char *subject) noexcept;
        void remove_subscription(const char *subject) noexcept;
        bool is_interested(const subject<> &subject) const noexcept;

        bool sendBuffer(const MessageBuffer::ByteStorageType &buffer) noexcept;
    };
}

#endif //FASTPATH_PEER_CONNECTION_H
