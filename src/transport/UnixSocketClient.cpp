//
// Created by fewstert on 01/04/16.
//

#include "UnixSocketClient.h"
#include "utils/logger.h"

namespace DCF {
    UnixSocketClient::UnixSocketClient(const std::string &path) : UnixSocket(path) {
    }

    UnixSocketClient::UnixSocketClient(UnixSocketClient &&other) noexcept : UnixSocket(std::move(other)) {
    }

    bool UnixSocketClient::connect(SocketOptions options) noexcept {
        if (::connect(m_socket, reinterpret_cast<struct sockaddr *>(&m_addr), sizeof(struct sockaddr_un)) != 0) {
            INFO_LOG("Failed to connect to IPC endpoint");
            return false;
        }

        m_connected = true;
        return true;
    }
}
