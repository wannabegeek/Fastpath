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

#ifndef FASTPATH_NOTIFIER_H
#define FASTPATH_NOTIFIER_H

#include <fcntl.h>
#include <sys/eventfd.h>
#include <atomic>
#include <cstring>
#include <unistd.h>

#include "fastpath/Exception.h"
#include "fastpath/event/EventType.h"
#include "fastpath/event/PollManager.h"

namespace fp {
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

        explicit notifier(int fd, direction d) noexcept {
            m_fd = fd;
        }

        notifier(notifier &&other) noexcept {
            m_fd = other.m_fd;
            other.m_fd = -1;
        }

        notifier(const notifier &other) = delete;
        notifier &operator=(const notifier &other) = delete;

        ~notifier() noexcept {
            if (m_fd != -1) {
                ::close(m_fd);
            }
        }

        inline void close_handle(direction d) noexcept {
            // no-op
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

        inline EventPollIOElement pollElement() const noexcept {
            return {m_fd, EventType::READ};
        }
    };
}

#endif //FASTPATH_NOTIFIER_H
