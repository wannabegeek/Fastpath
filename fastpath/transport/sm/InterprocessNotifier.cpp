//
// Created by fewstert on 01/04/16.
//
#ifdef HAVE_EVENTFD
#   include <sys/eventfd.h>
#endif
#include <fastpath/Exception.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <iostream>
#include <cassert>
#include "InterprocessNotifier.h"
#include "fastpath/transport/socket/UnixSocket.h"
#include "fastpath/utils/logger.h"

#define MAX_FDS 960

namespace fp {
    InterprocessNotifier::InterprocessNotifier(std::unique_ptr<UnixSocket> &&socket) : m_socket(std::move(socket)) {
    }


    bool InterprocessNotifier::send_fds(const int *fds, const size_t num_fds) noexcept {
        assert(num_fds < MAX_FDS);

        struct iovec iov;
        int data = ::getpid();
        iov.iov_base = &data;
        iov.iov_len = sizeof(int);

        struct msghdr msgh;
        char cmsgbuf[CMSG_SPACE(sizeof(int) * MAX_FDS)];

        msgh.msg_iov = &iov;
        msgh.msg_iovlen = 1;
        msgh.msg_name = NULL;
        msgh.msg_namelen = 0;
        msgh.msg_control = cmsgbuf;
        msgh.msg_controllen = CMSG_LEN(sizeof(int) * num_fds);

        struct cmsghdr *cmhp;
        cmhp = CMSG_FIRSTHDR(&msgh);
        cmhp->cmsg_len = CMSG_LEN(sizeof(int) * num_fds);
        cmhp->cmsg_level = SOL_SOCKET;
        cmhp->cmsg_type = SCM_RIGHTS;
        for (size_t i = 0; i < num_fds; i++) {
            INFO_LOG("Sending fd " << fds[i]);
            ((int *)CMSG_DATA(cmhp))[i] = fds[i];
        }

        if (!m_socket->send_ancillary(&msgh, 0)) {
            ERROR_LOG("sendmsg failed: " << strerror(errno));
            return false;
        }

        return true;
    }

    bool InterprocessNotifier::receive_fd(UnixSocket *socket, int *fds, size_t &num_fds, int &pid) noexcept {
        assert(num_fds < MAX_FDS);

        struct iovec iov;
        iov.iov_base = &pid;
        iov.iov_len = sizeof(int);

        struct msghdr msgh;
        union {
            struct cmsghdr cmh;
            int control[MAX_FDS];
        } control_un;

        msgh.msg_iov = &iov;
        msgh.msg_iovlen = 1;
        msgh.msg_name = NULL;
        msgh.msg_namelen = 0;

        msgh.msg_control = control_un.control;
        msgh.msg_controllen = sizeof(struct cmsghdr) + sizeof(int) * num_fds;

        fp::UnixSocket::ReadResult result = socket->read_ancillary(&msgh);
        if (result == fp::UnixSocket::MoreData) {
            struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msgh);
            if (cmsg != nullptr) {
                num_fds = std::min(num_fds, (cmsg->cmsg_len - sizeof(struct cmsghdr)) / sizeof(int));
                for (size_t i = 0; i < num_fds; i++) {
                    fds[i] = ((int *)CMSG_DATA(cmsg))[i];
                }

                return true;
            } else {
                ERROR_LOG("No header found");
            }
        } else if (result == fp::UnixSocket::NoData) {
            DEBUG_LOG("No data");
        } else if (result == fp::UnixSocket::Closed) {
            DEBUG_LOG("Closed");
        }

        return false;
    }
}

