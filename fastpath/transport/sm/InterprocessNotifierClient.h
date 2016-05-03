//
// Created by Tom Fewster on 03/04/2016.
//

#ifndef FASTPATH_INTERPROCESSNOTIFIERCLIENT_H
#define FASTPATH_INTERPROCESSNOTIFIERCLIENT_H

#include "InterprocessNotifier.h"
#include "fastpath/event/notifier.h"

namespace fp {
    class InterprocessNotifierClient : public InterprocessNotifier {
    private:
        fp::notifier inbound_notification;
        fp::notifier outbound_notification;

        bool m_connected = false;
    public:
        InterprocessNotifierClient() noexcept;

        inline bool is_connected() const noexcept { return m_connected; }

        bool connect() noexcept;
        bool notify() noexcept;
    };
}

#endif //FASTPATH_INTERPROCESSNOTIFIERCLIENT_H
