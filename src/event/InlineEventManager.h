//
// Created by Tom Fewster on 07/03/2016.
//

#ifndef TFDCF_INLINEEVENTMANAGER_H
#define TFDCF_INLINEEVENTMANAGER_H

#include "IOEvent.h"

namespace DCF {
    class Timer;
    class IOEvent;

    class InlineEventManager final : public EventManager {
    private:
        using IOEventTable = std::unordered_map<int, std::vector<IOEvent *>>;
        using TimerEventTable = std::unordered_map<int, TimerEvent *>;

        IOEventTable m_ioHandlerLookup;
        TimerEventTable m_timerHandlerLookup;

        mutable bool m_servicingEvents;
        mutable bool m_servicingTimers;

        void foreach_event_matching(const EventPollIOElement &event, std::function<void(IOEvent *)> callback) const override;
        void foreach_timer_matching(const EventPollTimerElement &event, std::function<void(TimerEvent *)> callback) const override;

        const bool haveHandlers() const override;
    public:
        InlineEventManager();
        InlineEventManager(InlineEventManager &&other);

        ~InlineEventManager();

        void registerHandler(TimerEvent *event) override;
        void registerHandler(IOEvent *event) override;
        void updateHandler(TimerEvent *event) override;
        void unregisterHandler(TimerEvent *event) override;
        void unregisterHandler(IOEvent *event) override;

        void notify(bool wait = false) override {}
    };
}

#endif //TFDCF_INLINEEVENTMANAGER_H
