//
//  TFSocket.h
//  TFFIXEngine
//
//  Created by Tom Fewster on 02/04/2013.
//  Copyright (c) 2013 Tom Fewster. All rights reserved.
//

#ifndef __TFFIXEngine__TFSocket__
#define __TFFIXEngine__TFSocket__

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <functional>
#include <atomic>
#include <utils/logger.h>

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

        Socket();
        Socket(Socket &&other);
        Socket(const int socketFd, const bool connected);

        virtual ~Socket();

        bool isConnected() const throw() {
            return m_connected;
        }

        int getSocket() {
            return m_socket;
        }

        void setSocket(const int socket, const bool isConnected = true);

        void setConnectionStateHandler(std::function<void(bool connected)> m_handler);

        virtual bool connect(SocketOptions options = SocketOptionsNone) noexcept {
            return false;
        };

        bool disconnect() throw();

        bool send(const char *data, ssize_t length) noexcept;
        bool send(const std::string &data) noexcept;
        bool send(const struct iovec *vec, const int &count) noexcept;

        const ReadResult read(std::string &data) noexcept;
        const ReadResult read(const char *data, size_t maxLength, ssize_t &length) noexcept;
    };
}
#endif /* defined(__TFFIXEngine__TFSocket__) */
