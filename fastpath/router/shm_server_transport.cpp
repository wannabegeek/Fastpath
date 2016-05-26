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

#include "shm_server_transport.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "fastpath/utils/logger.h"

#include "fastpath/transport/sm/InterprocessNotifierServer.h"
#include "fastpath/SharedMemoryBuffer.h"
#include "fastpath/event/Queue.h"
#include "fastpath/utils/temp_directory.h"
#include "fastpath/messages/MessageCodec.h"
#include "fastpath/router/shm_peer_connection.h"
#include "fastpath/router/message_wrapper.h"


namespace fp {
    shm_server_transport::shm_server_transport(Queue *queue, const ConnectionCallback &connectionCallback, const std::string &interface, const std::string &service) noexcept
            : server_transport(queue), m_manager(std::string("fprouter_").append(service).c_str(), true) {

        std::string ipc_file = tf::get_temp_directory();
        ipc_file.append("fprouter_");
        ipc_file.append(service);

        m_notification_server = std::make_unique<InterprocessNotifierServer>(ipc_file.c_str(), std::bind(&shm_server_transport::connectionHandler, this, connectionCallback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        m_globalConnectionHandler = m_notification_server->createReceiverEvent(m_queue);
    }

    shm_server_transport::~shm_server_transport() {
        m_queue->unregisterEvent(m_globalConnectionHandler);
    }

    void shm_server_transport::connectionHandler(const ConnectionCallback &connectionCallback, InterprocessNotifierServer::notifier_type &&notifier, std::unique_ptr<UnixSocket> &&socket, int process_id) {
        DEBUG_LOG("Someone connected to shared memory transport");

        auto *peer = new shm_peer_connection(m_queue, std::move(notifier), std::move(socket), process_id, &m_manager);
        connectionCallback(this, std::unique_ptr<peer_connection>(peer));
    }
}