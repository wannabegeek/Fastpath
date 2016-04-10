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
//        mutable bool m_servicingTimers;

        void foreach_event_matching(const EventPollIOElement &event, std::function<void(IOEvent *)> callback) const noexcept override;
        void foreach_timer_matching(const EventPollTimerElement &event, std::function<void(TimerEvent *)> callback) const noexcept override;

        const bool haveHandlers() const noexcept override;
    public:
        InlineEventManager() noexcept;
//        InlineEventManager(InlineEventManager &&other) noexcept;

        ~InlineEventManager() noexcept;

        void registerHandler(TimerEvent *event) noexcept override;
        void registerHandler(IOEvent *event) noexcept override;
        void updateHandler(TimerEvent *event) noexcept override;
        void unregisterHandler(TimerEvent *event) noexcept override;
        void unregisterHandler(IOEvent *event) noexcept override;

        void notify(bool wait = false) noexcept override {}
    };
}

#endif //TFDCF_INLINEEVENTMANAGER_H
