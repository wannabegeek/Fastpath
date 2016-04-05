//
// Created by fewstert on 01/04/16.
//

#ifndef TFDCF_INTERPROCESSNOTIFIER_H
#define TFDCF_INTERPROCESSNOTIFIER_H

#include <transport/UnixSocket.h>

#define READ_FD 0
#define WRITE_FD 1

namespace DCF {
    class UnixSocket;

    class InterprocessNotifier {
    protected:
        std::unique_ptr<UnixSocket> m_socket;

    public:
        InterprocessNotifier(std::unique_ptr<UnixSocket> &&socket);
        virtual ~InterprocessNotifier();

        bool send_fds(const int *fds, const size_t num_fds) noexcept;
        bool receive_fd(UnixSocket *socket, int *fd, size_t &num_fds) noexcept;
    };
}

#endif //TFDCF_INTERPROCESSNOTIFIER_H
