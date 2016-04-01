//
// Created by Tom Fewster on 20/03/2016.
//

#include <transport/Transport.h>
#include "Subscriber.h"

namespace DCF {
    bool Subscriber::is_interested(const fp::subject<> subject) const {
        return m_subscription.matches(subject);
    }

    void Subscriber::__dispatch(Transport::MessageType &message) const {
        m_callback(this, message.get());
        INFO_LOG("Dispatched " << message.use_count());
    }
}