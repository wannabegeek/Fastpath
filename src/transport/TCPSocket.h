//
// Created by Tom Fewster on 01/04/2016.
//

#ifndef TFDCF_TCPSOCKET_H
#define TFDCF_TCPSOCKET_H


#include <cstdint>
#include <functional>
#include "Socket.h"

namespace DCF {
    class TCPSocket : public Socket {
    protected:
        uint16_t m_port;
        struct hostent m_host;
        struct addrinfo *m_hostInfo = NULL;

    public:
        TCPSocket(const std::string &host, const std::string &service) throw(socket_error);
        TCPSocket(const std::string &host, const uint16_t &port) throw(socket_error);
        TCPSocket(const struct hostent *host, const uint16_t &port);
        TCPSocket(const int socketFd, const bool connected);

        virtual ~TCPSocket();

        void setOptions(int options) noexcept override;
    };
}


#endif //TFDCF_TCPSOCKET_H
