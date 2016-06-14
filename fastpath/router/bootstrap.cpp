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

#include "fastpath/event/IOEvent.h"
#include "fastpath/messages/Message.h"
#include "fastpath/router/bootstrap.h"
#include "fastpath/router/tcp_peer_connection.h"
#include "fastpath/router/shm_peer_connection.h"
#include "fastpath/router/tcp_server_transport.h"
#include "fastpath/router/shm_server_transport.h"

namespace fp {
    bootstrap::bootstrap(const std::string &interface, const std::string &service) : m_interface(interface), m_service(service) {
    }

    bootstrap::~bootstrap() {
    }

    void bootstrap::run() {
        // Start server socket listening

        auto peer_connection_handler = [&](server_transport *transport, std::unique_ptr<peer_connection> &&peer) noexcept {
            peer->setMessageHandler(std::bind(&bootstrap::message_handler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            peer->setDisconnectionHandler(std::bind(&bootstrap::disconnection_handler, this, std::placeholders::_1));
            m_connections.push_back(std::move(peer));
        };

        auto tcp_transport = std::make_unique<tcp_server_transport>(&m_dispatchQueue, peer_connection_handler, m_interface, m_service);
        auto shm_transport = std::make_unique<shm_server_transport>(&m_dispatchQueue, peer_connection_handler, m_interface, m_service);

        // TODO:
        // Start heartbeat thread
        // Start broadcast transport
        auto fn = [&] (fp::SignalEvent *event, int signal) {
            ERROR_LOG("Shutdown signal caught");
            m_shutdown = true;
        };

        auto sigint_ev = m_dispatchQueue.registerEvent(SIGINT, fn);
        auto sigterm_ev = m_dispatchQueue.registerEvent(SIGTERM, fn);

        while (!m_shutdown) {
            m_dispatchQueue.dispatch();
        }

        DEBUG_LOG("Shutting down");
        m_dispatchQueue.unregisterEvent(sigint_ev);
        m_dispatchQueue.unregisterEvent(sigterm_ev);
    }

    void bootstrap::message_handler(peer_connection *source, const subject<> &subject, const message_wrapper &msgData) noexcept {
        DEBUG_LOG("Processing message");
        // send the message out to all local client who are interested
        std::for_each(m_connections.begin(), m_connections.end(), [&](auto &connection) noexcept {
            if (connection->is_interested(subject)) {
                DEBUG_LOG("Connection is interested");
                // dispatch the message
                connection->sendBuffer(msgData);
            } else {
                DEBUG_LOG("Connection not interested");
            }
        });
    }

    void bootstrap::disconnection_handler(peer_connection *connection) noexcept {
        auto it = std::find_if(m_connections.begin(), m_connections.end(), [&](auto &c) noexcept {
            return c.get() == connection;
        });
        if (it != m_connections.end()) {
            DEBUG_LOG("Client has disconnected");
            m_connections.erase(it);
        } else {
            ERROR_LOG("Received disconnect notification for unknown connection");
        }
    }
}
