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

#include "fastpath/transport/realm_transport.h"
#include "fastpath/transport/TCPTransport.h"
#include "fastpath/transport/SHMTransport.h"

namespace fp {

    std::unique_ptr<DCF::Transport> make_relm_connection(const char *connection_url, const char *description) throw(fp::exception) {
        DCF::url url(connection_url);

        if (url.protocol() == "tcp") {
            return std::make_unique<DCF::TCPTransport>(url, description);
        } else if (url.protocol() == "shm") {
            return std::make_unique<DCF::SHMTransport>(url, description);
        } else {
            throw fp::exception("Unsupported protocol");
        }

        return nullptr;
    }

//
//    realm_transport::realm_transport(const char *url, const char *description) {
//        DCF::url u(url);
//
//        if (u.protocol() == "tcp") {
//            m_transport = std::make_unique<DCF::TCPTransport>(u, description);
//        } else if (u.protocol() == "ipc") {
//
//        } else if (u.protocol() == "shm") {
//
//        } else {
//            throw fp::exception("Unsupported protocol");
//        }
//
//
//        // add transport to the global event manager
//    }
//
//    bool realm_transport::set_queue(const DCF::Queue *queue) {
//        if (queue == nullptr) {
//            if (m_associatedQueue != nullptr) {
//                // we need to detach from the event manager
//            }
//        } else if (m_associatedQueue != nullptr) {
//            ERROR_LOG("Queue can only be associated with a single queue");
//            return false;
//        } else {
//            // we need to add our fd to the event manager of the queue
//        }
//        m_associatedQueue = queue;
//        return true;
//    }
//
//    void realm_transport::broadcastConnectionStateChange(bool connected) {
////        auto msg = std::make_shared<DCF::Message>();
////        msg->setSubject(connected ? subject::daemon_connected : subject::daemon_disconnected);
////        std::for_each(m_subscribers.begin(), m_subscribers.end(), [&](DCF::MessageListener *msgEvent) {
////            // TODO: check if the handler is interested in this message
////            msgEvent->__notify(msg);
////        });
//    }
//
//    const char *realm_transport::subject::daemon_connected = "_FP.INFO.DAEMON.CONNECTED";
//    const char *realm_transport::subject::daemon_disconnected = "_FP.ERROR.DAEMON.DISCONNECTED";
}