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

        mutable bool m_servicingEvents;
        mutable bool m_servicingTimers;

        bool m_pendingFileDescriptorRegistrationEvents;
        bool m_pendingTimerRegistrationEvents;
        std::vector<TimerEvent *> m_pendingTimerHandlers;

        std::vector<TimerEvent *> m_timerHandlers;
        IOEventTable m_ioHandlerLookup;

        void processPendingRegistrations() override;
        void foreach_timer(std::function<void(TimerEvent *)> callback) const override;
        void foreach_event_matching(const EventPollElement &event, std::function<void(IOEvent *)> callback) const override;

        const bool haveHandlers() const override;
    public:
        InlineEventManager();
        InlineEventManager(InlineEventManager &&other);

        ~InlineEventManager();

        void registerHandler(TimerEvent &eventRegistration) override;
        void registerHandler(IOEvent &eventRegistration) override;
        void unregisterHandler(TimerEvent &handler) override;
        void unregisterHandler(IOEvent &handler) override;

        void notify(bool wait = false) override {}
    };
}

#endif //TFDCF_INLINEEVENTMANAGER_H
