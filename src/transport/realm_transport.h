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
        constexpr const char *connected_subject() { return "_FP.INFO.DAEMON.CONNECTED"; }
        constexpr const char *disconnected_subject() { return "_FP.ERROR.DAEMON.DISCONNECTED"; }

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
