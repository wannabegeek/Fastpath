//
// Created by fewstert on 01/04/16.
//

#ifndef TFDCF_UNIXSOCKET_H
#define TFDCF_UNIXSOCKET_H

#include <atomic>
#include <sys/un.h>
#include <functional>
#include <transport/Socket.h>

namespace DCF {
    class UnixSocket : public Socket{
    protected:
        struct sockaddr_un m_addr;

    public:

        UnixSocket(const std::string &path) throw(socket_error);
        UnixSocket(const int socketFd, const bool connected);

        UnixSocket(UnixSocket &&other);

        virtual ~UnixSocket();

        void setOptions(int options) noexcept override;

        bool send_ancillary(const struct msghdr *msg, int flags) noexcept;
        const Socket::ReadResult read_ancillary(struct msghdr *msg) noexcept;
    };
}


#endif //TFDCF_UNIXSOCKET_H
