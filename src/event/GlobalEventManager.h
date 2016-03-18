//
// Created by Tom Fewster on 07/03/2016.
//

#ifndef TFDCF_GLOBALEVENTMANAGER_H
#define TFDCF_GLOBALEVENTMANAGER_H

#include <utils/concurrentqueue.h>
#include <utils/tfspinlock.h>
#include <unordered_map>
#include "EventManager.h"

namespace DCF {
    class Event;

    class GlobalEventManager final : public EventManager {
    private:
        using PendingTimerType = moodycamel::ConcurrentQueue<TimerEvent *>;
        using PendingIOType = moodycamel::ConcurrentQueue<IOEvent *>;
        using IOEventTable = std::unordered_map<int, std::vector<IOEvent *>>;

        ActionNotifier m_actionNotifier;
        PendingTimerType m_pendingTimerRegistrations;
        PendingIOType m_pendingIORegistrations;

        std::vector<TimerEvent *> m_timerHandlers;
        IOEventTable m_ioHandlerLookup;

        tf::spinlock m_lock;

        void serviceEvent(const EventPollElement &event) override;

        void processPendingRegistrations() override;
        void foreach_timer(std::function<void(TimerEvent *)> callback) const override;
        void foreach_event_matching(const EventPollElement &event, std::function<void(IOEvent *)> callback) const override;

        const bool haveHandlers() const override;
    public:
        GlobalEventManager();
        ~GlobalEventManager();

        void registerHandler(TimerEvent *eventRegistration) override;
        void registerHandler(IOEvent *eventRegistration) override;
        void unregisterHandler(TimerEvent *handler) override;
        void unregisterHandler(IOEvent *handler) override;

        void notify(bool wait = false) override;
    };
}

#endif //TFDCF_GLOBALEVENTMANAGER_H
