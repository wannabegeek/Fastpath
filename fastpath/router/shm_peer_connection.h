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

#ifndef FASTPATH_SHM_PEER_CONNECTION_H
#define FASTPATH_SHM_PEER_CONNECTION_H

#include "fastpath/router/peer_connection.h"
#include "fastpath/transport/sm/InterprocessNotifierServer.h"

namespace fp {
    class Queue;
    class SharedMemoryBuffer;
    class UnixSocket;
    class DataEvent;
    class SharedMemoryManager;

    class shm_peer_connection : public peer_connection {
        InterprocessNotifierServer::notifier_type m_notifier;

        std::unique_ptr<SharedMemoryBuffer> m_clientQueue;
        std::unique_ptr<SharedMemoryBuffer> m_serverQueue;

        std::unique_ptr<UnixSocket> m_socket;
        int m_process_id;

        DataEvent *m_socketEvent = nullptr;
        DataEvent *m_notifierEvent = nullptr;

//        std::function<void(PeerConnection *connection)> m_callback;
//        std::function<void(PeerConnection *connection)> m_msg_callback;
        void socketEvent();
        void notificationEvent();

        void notificationHandler(fp::DataEvent *event, const fp::EventType type) noexcept;
    public:

        shm_peer_connection(Queue *queue, InterprocessNotifierServer::notifier_type &&notifier, std::unique_ptr<UnixSocket> &&socket, int process_id, SharedMemoryManager *manager);
        shm_peer_connection(shm_peer_connection &&other) noexcept;
        ~shm_peer_connection() noexcept;

        bool sendBuffer(const message_wrapper &buffer) noexcept override;

        int get_process_id() const noexcept { return m_process_id; }
    };
}

#endif //FASTPATH_SHM_PEER_CONNECTION_H
