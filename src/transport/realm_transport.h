//
// Created by Tom Fewster on 16/03/2016.
//

#ifndef TFDCF_RELMTRANSPORT_H
#define TFDCF_RELMTRANSPORT_H

#include <vector>
#include <event/MessageEvent.h>

namespace fp {
    class realm_transport {
    private:
        std::vector<DCF::MessageEvent> m_subscribers;

        std::unique_ptr<DCF::Transport> m_transport;
        const DCF::Queue *m_associatedQueue;
    public:
        explicit realm_transport(const char *url, const char *description = "");
        virtual ~realm_transport() {}

        bool set_queue(const DCF::Queue *queue);
    };
}

#endif //TFDCF_RELMTRANSPORT_H
