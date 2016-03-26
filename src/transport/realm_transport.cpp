//
// Created by Tom Fewster on 16/03/2016.
//

#include "realm_transport.h"
#include "TCPTransport.h"

namespace fp {

    std::unique_ptr<DCF::Transport> make_relm_connection(const char *connection_url, const char *description) throw(fp::exception) {
        DCF::url url(connection_url);

        if (url.protocol() == "tcp") {
            return std::make_unique<DCF::TCPTransport>(url, description);
        } else if (url.protocol() == "ipc") {

        } else if (url.protocol() == "shm") {

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