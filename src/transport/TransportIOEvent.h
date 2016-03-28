//
// Created by fewstert on 23/03/16.
//

#ifndef TFDCF_TRANSPORTIOEVENT_H
#define TFDCF_TRANSPORTIOEVENT_H

#include <event/IOEvent.h>
#include <functional>

namespace DCF {
    class TransportIOEvent final : public IOEvent {
    private:
        const std::function<void(TransportIOEvent *event, const EventType type)> m_callback;
    public:
        TransportIOEvent(const int fd, const EventType &eventType, const std::function<void(TransportIOEvent *event, const EventType type)> &callback);

        const bool isEqual(const Event &other) const noexcept override;
        const bool __notify(const EventType &eventType) noexcept override;
        void __destroy() override;
    };
}

#endif //TFDCF_TRANSPORTIOEVENT_H
