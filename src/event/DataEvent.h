//
// Created by fewstert on 23/03/16.
//

#ifndef TFDCF_DATAEVENT_H
#define TFDCF_DATAEVENT_H

#include "IOEvent.h"

namespace DCF {
    class DataEvent final : public IOEvent {
    private:
        std::function<void(DataEvent *, const EventType)> m_callback;

        void dispatch(DataEvent *event, const EventType &eventType);

    public:
        DataEvent(Queue *queue, const int fd, const EventType eventType, const std::function<void(DataEvent *, const EventType)> &callback);
        DataEvent(DataEvent &&other) noexcept ;

        const bool isEqual(const Event &other) const noexcept override;
        const bool __notify(const EventType &eventType) noexcept override;
        void __destroy() override;
    };
}

#endif //TFDCF_DATAEVENT_H
