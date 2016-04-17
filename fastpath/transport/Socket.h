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

#ifndef __TFFIXEngine__TFSocket__
#define __TFFIXEngine__TFSocket__

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <functional>
#include <atomic>

#include "fastpath/utils/logger.h"

//#include "TFException.h"

namespace DCF {
    enum {
        SocketOptionsNone = 1 << 0,
        SocketOptionsNonBlocking = 1 << 1,
        SocketOptionsDisableNagle = 1 << 2,
        SocketOptionsDisableSigPipe = 1 << 3,
        SocketOptionsHighPriority = 1 << 4
    };

    using SocketOptions = uint32_t;

    class socket_error : public std::logic_error{
    public:
        socket_error(const std::string &reason) : std::logic_error(reason) {
        };
    };

    class Socket {
    protected:
        int m_socket;
        std::atomic<bool> m_connected;

        int m_options;

        // we are uncopyable
        Socket(const Socket &socket) = delete;
        // ...and un assignable
        Socket &operator=(const Socket &rval) = delete;

        std::function<void(bool connected)> m_handler;

        virtual void setOptions(int options) noexcept = 0;

    public:
        typedef enum {
            MoreData,
            NoData,
            Closed
        } ReadResult;

        Socket() noexcept;
        Socket(Socket &&other) noexcept;
        Socket(const int socketFd, const bool connected) noexcept;

        virtual ~Socket() noexcept;

        bool isConnected() const throw() {
            return m_connected;
        }

        int getSocket() const noexcept{
            return m_socket;
        }

        void setSocket(const int socket, const bool isConnected = true) noexcept;

        void setConnectionStateHandler(std::function<void(bool connected)> m_handler) noexcept;

        virtual bool connect(SocketOptions options = SocketOptionsNone) noexcept {
            return false;
        };

        bool disconnect() noexcept;

        bool send(const char *data, ssize_t length) noexcept;
        bool send(const std::string &data) noexcept;
        bool send(const struct iovec *vec, const int &count) noexcept;

        const ReadResult read(std::string &data) noexcept;
        const ReadResult read(const char *data, size_t maxLength, ssize_t &length) noexcept;
    };
}
#endif /* defined(__TFFIXEngine__TFSocket__) */
