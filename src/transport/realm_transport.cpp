//
// Created by Tom Fewster on 16/03/2016.
//

#include "realm_transport.h"
#include "TCPTransport.h"

namespace fp {

    realm_transport::realm_transport(const char *url, const char *description) : DCF::TCPTransport(url, description) {
        m_peer->setConnectionStateHandler([&](bool connected) {
            INFO_LOG("Transport connected: " << std::boolalpha << connected);
            this->broadcastConnectionStateChange(connected);

            if (!m_shouldDisconnect) {
                this->__connect();
            }
        });

        // add transport to the global event manager
    }

    bool realm_transport::set_queue(const DCF::Queue *queue) {
        if (queue == nullptr) {
            if (m_associatedQueue != nullptr) {
                // we need to detach from the event manager
            }
        } else if (m_associatedQueue != nullptr) {
            ERROR_LOG("Queue can only be associated with a single queue");
            return false;
        } else {
            // we need to add our fd to the event manager of the queue
        }
        m_associatedQueue = queue;
        return true;
    }

    void realm_transport::broadcastConnectionStateChange(bool connected) {
        auto msg = std::make_shared<DCF::Message>();
        msg->setSubject(connected ? subject::daemon_connected : subject::daemon_disconnected);
        std::for_each(m_subscribers.begin(), m_subscribers.end(), [&](DCF::MessageEvent *msgEvent) {
            // TODO: check if the handler is interested in this message
            msgEvent->__notify(msg);
        });
    }

    const char *realm_transport::subject::daemon_connected = "_FP.INFO.DAEMON.CONNECTED";
    const char *realm_transport::subject::daemon_disconnected = "_FP.INFO.DAEMON.DISCONNECTED";
}