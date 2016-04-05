//
// Created by Tom Fewster on 03/04/2016.
//

#ifndef TFDCF_INTERPROCESSNOTIFIERSERVER_H
#define TFDCF_INTERPROCESSNOTIFIERSERVER_H

#include <event/EventType.h>
#include <transport/TransportIOEvent.h>
#include "InterprocessNotifier.h"
#include "notifier.h"

namespace DCF {
    class InterprocessNotifierServer : public InterprocessNotifier {
    private:
        std::function<void(tf::notifier &&notifier)> m_callback;
    public:
        InterprocessNotifierServer(std::function<void(tf::notifier &&notifier)> callback);

        virtual std::unique_ptr<TransportIOEvent> createReceiverEvent();
    };
}


#endif //TFDCF_INTERPROCESSNOTIFIERSERVER_H
