//
// Created by fewstert on 01/04/16.
//

#ifndef FASTPATH_INTERPROCESSNOTIFIER_H
#define FASTPATH_INTERPROCESSNOTIFIER_H

#include <fastpath/transport/UnixSocket.h>

#define READ_FD 0
#define WRITE_FD 1

namespace fp {
    class UnixSocket;

    class InterprocessNotifier {
    protected:
        std::unique_ptr<UnixSocket> m_socket;

    public:
        InterprocessNotifier(std::unique_ptr<UnixSocket> &&socket);
        virtual ~InterprocessNotifier() = default;

        bool send_fds(const int *fds, const size_t num_fds) noexcept;
        bool receive_fd(UnixSocket *socket, int *fd, size_t &num_fds) noexcept;
    };
}

#endif //FASTPATH_INTERPROCESSNOTIFIER_H
