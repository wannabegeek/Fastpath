//
// Created by Tom Fewster on 03/04/2016.
//

#include <fastpath/transport/UnixSocketClient.h>
#include <fastpath/utils/logger.h>
#include "InterprocessNotifierClient.h"

namespace fp {
    InterprocessNotifierClient::InterprocessNotifierClient() : InterprocessNotifier(std::make_unique<UnixSocketClient>("test_unix")) {
    }

    bool InterprocessNotifierClient::connect() noexcept {
        if (!m_connected && m_socket->connect(fp::SocketOptionsNone)) {
            int p[] = {outbound_notification.read_handle(), inbound_notification.signal_handle()};
            this->send_fds(p, 2);
            //            ::close(outbound_notification.read_handle());
            //            ::close(inbound_notification.signal_handle());
            m_connected = true;
            return true;
        }

        return false;
    }

    bool InterprocessNotifierClient::notify() noexcept {
        return outbound_notification.notify();
    }
}

