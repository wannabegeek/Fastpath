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

    class SocketException { //: public Exception {
    public:
        SocketException(const std::string &reason) { //: Exception("SocketException", reason) {
        };
    };

    class Socket {
    protected:
        uint16_t m_port;
        struct hostent m_host;

        struct addrinfo *m_hostInfo = NULL;

        int m_socket;
        std::atomic<bool> m_connected;

        int m_options;

        // we are uncopyable
        Socket(const Socket &socket) = delete;
        // ...and un assignable
        Socket &operator=(const Socket &rval) = delete;

        std::function<void(bool connected)> m_handler;

        //	TFSocket(const std::string &host, const std::string &service, int socketFd) throw(TFSocketException); // initialise a connected socket

    protected:
        void setOptions(int options);

    public:
        typedef enum {
            MoreData,
            NoData,
            Closed
        } ReadResult;

        Socket(const std::string &host, const std::string &service) throw(SocketException);
        Socket(const std::string &host, const uint16_t &port) throw(SocketException);
        Socket(const struct hostent *host, const uint16_t &port);
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
