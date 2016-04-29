//
// Created by Tom Fewster on 03/04/2016.
//

#ifndef FASTPATH_INTERPROCESSNOTIFIERSERVER_H
#define FASTPATH_INTERPROCESSNOTIFIERSERVER_H

#include <fastpath/event/EventType.h>
#include <fastpath/transport/TransportIOEvent.h>
#include "InterprocessNotifier.h"
#include "fastpath/event/notifier.h"

namespace fp {
    class InterprocessNotifierServer : public InterprocessNotifier {
    private:
        std::function<void(std::unique_ptr<fp::notifier> &&notifier)> m_callback;
    public:
        InterprocessNotifierServer(std::function<void(std::unique_ptr<fp::notifier> &&notifier)> callback);

        virtual std::unique_ptr<TransportIOEvent> createReceiverEvent();
    };
}


#endif //FASTPATH_INTERPROCESSNOTIFIERSERVER_H
