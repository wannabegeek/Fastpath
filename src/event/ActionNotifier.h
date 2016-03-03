//
// Created by Tom Fewster on 02/03/2016.
//

#ifndef TFDCF_ACTIONNOTIFIER_H
#define TFDCF_ACTIONNOTIFIER_H

#include <unistd.h>
#include "PollManager.h"

namespace DCF {
    class ActionNotifier {
    private:
        int m_fd[2];

    public:
        explicit ActionNotifier() {
            ::pipe(m_fd);
        }

        ~ActionNotifier() {
            close(m_fd[0]);
            close(m_fd[1]);
        }

        inline void notify() {
            char data[] = "\n";
            unsigned int length = 1;
            write(m_fd[1], data, length);
        }

        inline void reset() {
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
}

#endif //TFDCF_ACTIONNOTIFIER_H