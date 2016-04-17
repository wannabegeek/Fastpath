/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef TFDCF_ACTIONNOTIFIER_H
#define TFDCF_ACTIONNOTIFIER_H

#include <unistd.h>
#include <atomic>

#include "fastpath/config.h"

#ifdef HAVE_EVENTFD
#include <sys/eventfd.h>
#else
#include <fcntl.h>
#endif

#include "fastpath/Exception.h"
#include "fastpath/event/PollManager.h"
#include "fastpath/event/EventType.h"

namespace DCF {
#ifdef HAVE_EVENTFD
    class ActionNotifier {
    private:
        int m_fd;
        std::atomic_flag m_locked = ATOMIC_FLAG_INIT;

    public:
        explicit ActionNotifier() {
            m_fd = eventfd(0, O_NONBLOCK);
            if (m_fd == -1) {
                ThrowException(fp::exception, "Failed to create eventfd: " << strerror(errno));
            }
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
            read(m_fd, &data, sizeof(uint64_t));
        }

        inline int read_handle() const noexcept {
            return m_fd;
        }

        inline EventPollIOElement pollElement() const {
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
            if (::pipe(m_fd) == -1) {
                ThrowException(fp::exception, "Failed to create pipe: " << strerror(errno));
            }
            ::fcntl(m_fd[0], F_SETFD, O_NONBLOCK);
            ::fcntl(m_fd[1], F_SETFD, O_NONBLOCK);
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

        inline EventPollIOElement pollElement() const {
            return {m_fd[0], EventType::READ};
        }
    };
#endif
}

#endif //TFDCF_ACTIONNOTIFIER_H
