//
// Created by Tom Fewster on 03/04/2016.
//

#ifndef FASTPATH_INTERPROCESSNOTIFIERCLIENT_H
#define FASTPATH_INTERPROCESSNOTIFIERCLIENT_H

#include "InterprocessNotifier.h"
#include "notifier.h"

namespace fp {
    class InterprocessNotifierClient : public InterprocessNotifier {
    private:
        tf::notifier inbound_notification;
        tf::notifier outbound_notification;

    public:
        InterprocessNotifierClient();

        bool notify() noexcept;
    };
}

#endif //FASTPATH_INTERPROCESSNOTIFIERCLIENT_H
