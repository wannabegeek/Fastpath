//
// Created by fewstert on 01/04/16.
//

#ifndef TFDCF_INTERPROCESSNOTIFIER_H
#define TFDCF_INTERPROCESSNOTIFIER_H

#include <transport/UnixSocket.h>

namespace DCF {
    class UnixSocket;

    class InterprocessNotifier {
    protected:
        int m_fd = -1;
        std::unique_ptr<UnixSocket> m_socket;

    public:
        InterprocessNotifier(std::unique_ptr<UnixSocket> &&socket);
        virtual ~InterprocessNotifier();

        int getFileDescriptor() const noexcept { return m_fd; }

        bool send_fds(const int *fds, const size_t num_fds) noexcept;
        bool receive_fd(int *fd, size_t &num_fds) noexcept;

        bool notify() const noexcept;
    };
}

#endif //TFDCF_INTERPROCESSNOTIFIER_H
