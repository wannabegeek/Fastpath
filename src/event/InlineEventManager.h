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
        bool m_pendingFileDescriptorRegistrationEvents;
        bool m_pendingTimerRegistrationEvents;
        std::vector<TimerEvent *> m_pendingTimerHandlers;
        std::vector<IOEvent *> m_pendingHandlers;

        std::vector<TimerEvent *> m_timerHandlers;
        std::vector<IOEvent *> m_ioHandlers;

        void processPendingRegistrations() override;
        void __foreach_timer(std::function<void(TimerEvent *)> callback) const override;
        void __foreach_ioevent(std::function<void(IOEvent *)> callback) const override;

        const bool haveHandlers() const override;
    public:
        InlineEventManager();
        InlineEventManager(InlineEventManager &&other);

        ~InlineEventManager();

        void registerHandler(TimerEvent &eventRegistration) override;
        void registerHandler(IOEvent &eventRegistration) override;
        void unregisterHandler(TimerEvent &handler) override;
        void unregisterHandler(IOEvent &handler) override;

        void notify() override {}
    };
}

#endif //TFDCF_INLINEEVENTMANAGER_H
