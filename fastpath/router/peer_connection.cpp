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

#include "peer_connection.h"

namespace fp {
    peer_connection::peer_connection(Queue *queue)
        : m_queue(queue) {
    }

    peer_connection::peer_connection(peer_connection &&other) noexcept
            : m_subscriptions(std::move(other.m_subscriptions)),
              m_queue(other.m_queue),
              m_disconnectionHandler(other.m_disconnectionHandler) {
    }

    peer_connection::~peer_connection() noexcept {
        DEBUG_LOG("Peer connection destroyed");
    }


    void peer_connection::setMessageHandler(const MessageHandlerType &messageHandler) {
        m_messageHandler = messageHandler;
    }

    void peer_connection::setDisconnectionHandler(const DisconnectionHandlerType &disconnectionHandler) {
        m_disconnectionHandler = disconnectionHandler;
    }

    void peer_connection::add_subscription(const char *subject) noexcept {
        m_subscriptions.emplace_back(subject);
    }

    void peer_connection::remove_subscription(const char *subject) noexcept {
        auto it = std::find_if(m_subscriptions.begin(), m_subscriptions.end(), [&](const subscription<> &s) noexcept {
            return s == subject;
        });
        if (it != m_subscriptions.end()) {
            m_subscriptions.erase(it);
        }
    }

    bool peer_connection::is_interested(const subject<> &subject) const noexcept {
        auto it = std::find_if(m_subscriptions.begin(), m_subscriptions.end(), [&](const subscription<> &s) noexcept {
            return s.matches(subject);
        });

        return (it != m_subscriptions.end());
    }

    void peer_connection::handle_admin_message(const subject<> subject, Message &message) noexcept {
        DEBUG_LOG("Received admin msg: " << message);

        if (subject == RegisterObserver()) {
            const char *data = nullptr;
            size_t len = 0;
            if (!message.getDataField("subject", &data, len) || len == 0) {
                ERROR_LOG("Received invalid message - subscription without a subject to subscribe to");
                return;
            }
            this->add_subscription(data);
        } else if (subject == RegisterObserver()) {
            const char *data = nullptr;
            size_t len = 0;
            if (!message.getDataField("subject", &data, len) && len > 0) {
                ERROR_LOG("Received invalid message - subscription without a subject to subscribe to");
                return;
            }
            this->remove_subscription(data);
        }
    }
}