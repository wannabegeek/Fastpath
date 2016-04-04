//
// Created by Tom Fewster on 03/04/2016.
//

#ifndef TFDCF_INTERPROCESSNOTIFIERSERVER_H
#define TFDCF_INTERPROCESSNOTIFIERSERVER_H


#include <event/EventType.h>
#include <transport/TransportIOEvent.h>
#include "InterprocessNotifier.h"

namespace DCF {
    class InterprocessNotifierServer : public InterprocessNotifier {
    public:
        InterprocessNotifierServer();

        virtual std::unique_ptr<TransportIOEvent> createReceiverEvent();
    };
}


#endif //TFDCF_INTERPROCESSNOTIFIERSERVER_H
