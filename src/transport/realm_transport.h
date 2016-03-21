//
// Created by Tom Fewster on 16/03/2016.
//

#ifndef TFDCF_RELMTRANSPORT_H
#define TFDCF_RELMTRANSPORT_H

#include <vector>
#include "event/MessageListener.h"
#include <transport/TCPTransport.h>
#include <event/Queue.h>

namespace fp {
    class Transport;

    std::unique_ptr<DCF::Transport> make_relm_connection(const char *connection_url, const char *description = "") throw(fp::exception);

    class realm_transport {
    private:
        struct subject {
            static const char *daemon_connected;
            static const char *daemon_disconnected;
        };

        std::vector<DCF::MessageListener *> m_subscribers;
        const DCF::Queue *m_associatedQueue;

        std::unique_ptr<DCF::Transport> m_transport;

        void broadcastConnectionStateChange(bool connected);
    public:
        explicit realm_transport(const char *url, const char *description = "");
        virtual ~realm_transport() {}

        bool set_queue(const DCF::Queue *queue);
    };
}

#endif //TFDCF_RELMTRANSPORT_H
