//
// Created by Tom Fewster on 05/04/2016.
//

#ifndef TFDCF_NOTIFIER_H
#define TFDCF_NOTIFIER_H

#include <sys/eventfd.h>

namespace tf {
    class notifier {
    private:
        int m_fd;
    public:
        explicit notifier() {
            m_fd = eventfd(0, O_NONBLOCK);
            if (m_fd == -1) {
                ThrowException(fp::exception, "Failed to create eventfd: " << strerror(errno));
            }
        }

        explicit notifier(int *fd) noexcept {
            m_fd = fd[0];
        }

        notifier(notifier &&other) noexcept {
            m_fd = other.m_fd;
        }

        ~notifier() noexcept {
            ::close(m_fd);
        }

        inline bool notify() noexcept {
            const uint64_t data = 1;
            return (::write(m_fd, &data, sizeof(uint64_t)) != -1);
        }

        inline bool reset() noexcept {
            uint64_t data;
            return (::read(m_fd, &data, sizeof(uint64_t)) != -1);
        }

        inline int read_handle() const noexcept {
            return m_fd;
        }

        inline int signal_handle() const noexcept {
            return m_fd;
        }
    };
}

#endif //TFDCF_NOTIFIER_H
