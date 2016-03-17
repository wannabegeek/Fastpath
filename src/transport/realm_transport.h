//
// Created by Tom Fewster on 16/03/2016.
//

#ifndef TFDCF_RELMTRANSPORT_H
#define TFDCF_RELMTRANSPORT_H

#include <vector>
#include <event/MessageEvent.h>
#include <transport/TCPTransport.h>

namespace fp {
    class realm_transport : public DCF::TCPTransport {
    private:
        struct subject {
            static const char *daemon_connected;
            static const char *daemon_disconnected;
        };

        std::vector<DCF::MessageEvent *> m_subscribers;
        const DCF::Queue *m_associatedQueue;

        void broadcastConnectionStateChange(bool connected);
    public:
        explicit realm_transport(const char *url, const char *description = "");
        virtual ~realm_transport() {}

        bool set_queue(const DCF::Queue *queue);
    };
}

#endif //TFDCF_RELMTRANSPORT_H
