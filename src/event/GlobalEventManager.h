//
// Created by Tom Fewster on 07/03/2016.
//

#ifndef TFDCF_GLOBALEVENTMANAGER_H
#define TFDCF_GLOBALEVENTMANAGER_H

#include <utils/tfspinlock.h>
#include <unordered_map>
#include <utils/rwlock.h>
#include "EventManager.h"

namespace DCF {
    class Event;

    class GlobalEventManager final : public EventManager {
    private:
        using IOEventTable = std::unordered_map<int, std::vector<IOEvent *>>;
        using TimerEventTable = std::unordered_map<int, TimerEvent *>;

        ActionNotifier m_actionNotifier;

        IOEventTable m_ioHandlerLookup;
        TimerEventTable m_timerHandlerLookup;

        mutable tf::rwlock m_lock;

        void foreach_event_matching(const EventPollIOElement &event, std::function<void(IOEvent *)> callback) const override;
        void foreach_timer_matching(const EventPollTimerElement &event, std::function<void(TimerEvent *)> callback) const override;

        const bool haveHandlers() const override;
    public:
        GlobalEventManager();
        ~GlobalEventManager();

        void registerHandler(TimerEvent *eventRegistration) override;
        void registerHandler(IOEvent *eventRegistration) override;
        void updateHandler(TimerEvent *eventRegistration) override;
        void unregisterHandler(TimerEvent *handler) override;
        void unregisterHandler(IOEvent *handler) override;

        void notify(bool wait = false) override;
    };
}

#endif //TFDCF_GLOBALEVENTMANAGER_H
