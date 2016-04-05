//
// Created by Tom Fewster on 03/04/2016.
//

#ifndef TFDCF_INTERPROCESSNOTIFIERCLIENT_H
#define TFDCF_INTERPROCESSNOTIFIERCLIENT_H

#include "InterprocessNotifier.h"
#include "notifier.h"

namespace DCF {
    class InterprocessNotifierClient : public InterprocessNotifier {
    private:
        tf::notifier inbound_notification;
        tf::notifier outbound_notification;

    public:
        InterprocessNotifierClient();

        bool notify() noexcept;
    };
}

#endif //TFDCF_INTERPROCESSNOTIFIERCLIENT_H
