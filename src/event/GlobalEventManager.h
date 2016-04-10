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

        void foreach_event_matching(const EventPollIOElement &event, std::function<void(IOEvent *)> callback) const noexcept override;
        void foreach_timer_matching(const EventPollTimerElement &event, std::function<void(TimerEvent *)> callback) const noexcept override;

        const bool haveHandlers() const noexcept override;
    public:
        GlobalEventManager();
        ~GlobalEventManager();

        void registerHandler(TimerEvent *eventRegistration) noexcept override;
        void registerHandler(IOEvent *eventRegistration) noexcept override;
        void updateHandler(TimerEvent *eventRegistration) noexcept override;
        void unregisterHandler(TimerEvent *handler) noexcept override;
        void unregisterHandler(IOEvent *handler) noexcept override;

        void notify(bool wait = false) noexcept override;
    };
}

#endif //TFDCF_GLOBALEVENTMANAGER_H
