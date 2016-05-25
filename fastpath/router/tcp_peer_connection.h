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

#ifndef FASTPATH_TCP_PEER_CONNECTION_H
#define FASTPATH_TCP_PEER_CONNECTION_H

#include <string>
#include <vector>
#include <memory>

#include "fastpath/messages/MessageCodec.h"
#include "fastpath/event/IOEvent.h"
#include "fastpath/messages/Message.h"
#include "fastpath/MessageBuffer.h"
#include "fastpath/messages/subscription.h"
#include "fastpath/router/peer_connection.h"

namespace fp {
    class Socket;
}

namespace fp {
    class tcp_peer_connection final : public peer_connection {
    private:
        std::unique_ptr<Socket> m_socket;
        DataEvent *m_socketEvent;

        MessageBuffer m_buffer;

        void data_handler(DataEvent *event, const EventType eventType) noexcept;
        MessageCodec::MessageDecodeStatus process_buffer(const MessageBuffer::ByteStorageType &buffer) noexcept;

    public:
        tcp_peer_connection(Queue *queue, std::unique_ptr<Socket> socket);
        tcp_peer_connection(tcp_peer_connection &&other) noexcept;
        ~tcp_peer_connection() noexcept;

        bool sendBuffer(const message_wrapper &buffer) noexcept override;
    };
}

#endif //FASTPATH_PEER_CONNECTION_H
