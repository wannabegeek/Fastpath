//
// Created by Tom Fewster on 16/03/2016.
//

#include "realm_transport.h"
#include "TCPTransport.h"

namespace fp {

    realm_transport::realm_transport(const char *url, const char *description) {
        m_transport = std::make_unique<DCF::TCPTransport>(url, description);
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

}