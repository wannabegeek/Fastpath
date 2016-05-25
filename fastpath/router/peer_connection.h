/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 26/03/2016

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
#include <functional>

#include "fastpath/messages/subscription.h"
#include "fastpath/messages/Message.h"

namespace fp {
    class Queue;
    class message_wrapper;

    class peer_connection {
    public:
        typedef std::function<void(peer_connection *, const subject<> &, const message_wrapper &)> MessageHandlerType;
        typedef std::function<void(peer_connection *)> DisconnectionHandlerType;
    private:
        // TODO: these need to be calculated up-front
        static subject<> RegisterObserver() { return subject<>("_FP.REGISTER.OBSERVER"); }
        static subject<> UnregisterObserver() { return subject<>("_FP.UNREGISTER.OBSERVER"); }

        std::vector<subscription<>> m_subscriptions;

    protected:
        Queue *m_queue;

        MessageHandlerType m_messageHandler;
        DisconnectionHandlerType m_disconnectionHandler;

        void handle_admin_message(const subject<> subject, Message &message) noexcept;
    public:
        peer_connection(Queue *queue);
        peer_connection(peer_connection &&other) noexcept;
        virtual ~peer_connection() noexcept;

        void setMessageHandler(const MessageHandlerType &messageHandler);
        void setDisconnectionHandler(const DisconnectionHandlerType &disconnectionHandler);

        void add_subscription(const char *subject) noexcept;
        void remove_subscription(const char *subject) noexcept;
        bool is_interested(const subject<> &subject) const noexcept;

        virtual bool sendBuffer(const message_wrapper &buffer) noexcept = 0;
    };
}

#endif //FASTPATH_PEER_CONNECTION_H
