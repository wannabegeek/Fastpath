//
// Created by Tom Fewster on 05/04/2016.
//

#ifndef TFDCF_NOTIFIER_H
#define TFDCF_NOTIFIER_H

#include <unistd.h>
#include <fcntl.h>

#include "../../../../src/config.h"
#include <Exception.h>

namespace tf {
    class notifier {
    private:
        int m_fd[2];
    public:
        explicit notifier() {
#ifdef HAVE_PIPE2
            if (::pipe2(m_fd, O_NONBLOCK) == -1) {
                ThrowException(fp::exception, "Failed to create pipe: " << strerror(errno));
            }
#else
            if (::pipe(m_fd) == -1) {
                ThrowException(fp::exception, "Failed to create pipe: " << strerror(errno));
            }

            fcntl(m_fd[0], F_SETFL, fcntl(m_fd[0], F_GETFL) | O_NONBLOCK);
            fcntl(m_fd[1], F_SETFL, fcntl(m_fd[1], F_GETFL) | O_NONBLOCK);
#endif
        }

        explicit notifier(int *fd) noexcept {
            m_fd[0] = fd[0];
            m_fd[1] = fd[1];
        }

        notifier(notifier &&other) noexcept {
            m_fd[0] = other.m_fd[0];
            m_fd[1] = other.m_fd[1];

            other.m_fd[0] = -1;
            other.m_fd[1] = -1;
        }

        ~notifier() noexcept {
            if (m_fd[0] != -1) {
                ::close(m_fd[0]);
            }
            if (m_fd[1] != -1) {
                ::close(m_fd[1]);
            }
        }

        inline bool notify() noexcept {
            const char data = '=';
            return (::write(m_fd[1], &data, 1) != -1);
        }

        inline bool reset() noexcept {
            char data[256];
            unsigned int length = 255;
            return (::read(m_fd[0], &data, length) > 0);
        }

        inline int read_handle() const noexcept {
            return m_fd[0];
        }

        inline int signal_handle() const noexcept {
            return m_fd[1];
        }
    };
}

#endif //TFDCF_NOTIFIER_H
