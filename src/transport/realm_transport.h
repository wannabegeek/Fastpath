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

}

#endif //TFDCF_RELMTRANSPORT_H
