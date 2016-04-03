//
// Created by Tom Fewster on 03/04/2016.
//

#include <transport/UnixSocketClient.h>
#include <utils/logger.h>
#include "InterprocessNotifierClient.h"

namespace DCF {
    InterprocessNotifierClient::InterprocessNotifierClient() : InterprocessNotifier(std::make_unique<UnixSocketClient>("test_unix")) {
        if (m_socket->connect(DCF::SocketOptionsNone)) {
            int p[2];
            ::pipe(p);
            this->send_fds(p, 2);

        } else {
            ERROR_LOG("Failed to connect");
        }
    }
}

