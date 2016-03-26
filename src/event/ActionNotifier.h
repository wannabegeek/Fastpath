//
// Created by Tom Fewster on 02/03/2016.
//

#ifndef TFDCF_ACTIONNOTIFIER_H
#define TFDCF_ACTIONNOTIFIER_H

#include <unistd.h>
#include <atomic>
#include "PollManager.h"

namespace DCF {
#ifdef HAVE_EVENTFD
    class ActionNotifier {
    private:
        unsigned int m_fd;
        std::atomic_flag m_locked = ATOMIC_FLAG_INIT;

    public:
        explicit ActionNotifier() {
            m_fd = eventfd(0, O_NONBLOCK);
        }

        ActionNotifier(ActionNotifier &&other) {
            m_fd = other.m_fd;
        }

        ~ActionNotifier() {
            close(m_fd);
        }

        inline void notify_and_wait() {
            m_locked.test_and_set(std::memory_order_acquire);
            this->notify();
            while(!m_locked.test_and_set(std::memory_order_acquire));
        }

        inline void notify() {
            const uint64_t data = 1;
            write(m_fd, &data, sizeof(uint64_t));
        }

        inline void reset() {
            m_locked.clear(std::memory_order_release);
            uint64_t data;
            while (read(m_fd, &data, sizeof(uint64_t)) != EAGAIN);
        }

        inline int read_handle() const noexcept {
            return m_fd;
        }

        inline EventPollElement pollElement() const {
            return {m_fd, EventType::READ};
        }
    };

#else
    class ActionNotifier {
    private:
        int m_fd[2];

        std::atomic_flag m_locked = ATOMIC_FLAG_INIT;

    public:
        explicit ActionNotifier() {
            ::pipe(m_fd);
        }

        ActionNotifier(ActionNotifier &&other) {
            m_fd[0] = other.m_fd[0];
            m_fd[1] = other.m_fd[1];
        }

        ~ActionNotifier() {
            close(m_fd[0]);
            close(m_fd[1]);
        }

        inline void notify_and_wait() {
            m_locked.test_and_set(std::memory_order_acquire);
            this->notify();
            while(!m_locked.test_and_set(std::memory_order_acquire));
        }

        inline void notify() {
            char data[] = "\n";
            unsigned int length = 1;
            write(m_fd[1], data, length);
        }

        inline void reset() {
            m_locked.clear(std::memory_order_release);
            char data[256];
            unsigned int length = 255;
            read(m_fd[0], data, length);
        }

        inline int read_handle() const noexcept {
            return m_fd[0];
        }

        inline EventPollElement pollElement() const {
            return {m_fd[0], EventType::READ};
        }
    };
#endif
}

#endif //TFDCF_ACTIONNOTIFIER_H
