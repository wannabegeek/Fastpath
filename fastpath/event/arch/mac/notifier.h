//
// Created by Tom Fewster on 05/04/2016.
//

#ifndef FASTPATH_NOTIFIER_H
#define FASTPATH_NOTIFIER_H

#include <unistd.h>
#include <fcntl.h>
#include <atomic>
#include <fastpath/utils/logger.h>

#include "fastpath/config.h"
#include "fastpath/Exception.h"
#include "fastpath/event/EventType.h"
#include "fastpath/event/PollManager.h"

namespace fp {
    class notifier {
    private:
        int m_fd[2];
        std::atomic_flag m_locked = ATOMIC_FLAG_INIT;

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

            if (fcntl(m_fd[0], F_SETFL, fcntl(m_fd[0], F_GETFL) | O_NONBLOCK) != 0) {
                ERROR_LOG("Failed to set O_NONBLOCK on m_fd[0]");
            }

            if (fcntl(m_fd[1], F_SETFL, fcntl(m_fd[1], F_GETFL) | O_NONBLOCK) != 0) {
                ERROR_LOG("Failed to set O_NONBLOCK on m_fd[1]");
            }
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
            m_fd[0] = -1;
            m_fd[1] = -1;
        }

        inline bool notify() noexcept {
            const char data = '=';
            return (::write(m_fd[1], &data, 1) != -1);
        }

        inline void notify_and_wait() noexcept {
            m_locked.test_and_set(std::memory_order_acquire);
            this->notify();
            while(!m_locked.test_and_set(std::memory_order_acquire));
        }

        inline bool reset() noexcept {
            m_locked.clear(std::memory_order_release);
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

        inline EventPollIOElement pollElement() const noexcept {
            return {m_fd[0], EventType::READ};
        }

    };
}

#endif //FASTPATH_NOTIFIER_H
