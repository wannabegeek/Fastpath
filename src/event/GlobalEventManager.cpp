//
// Created by Tom Fewster on 07/03/2016.
//

#include "GlobalEventManager.h"
#include "TimerEvent.h"
#include "IOEvent.h"

namespace DCF {

    GlobalEventManager::GlobalEventManager() {
        m_eventLoop.add(m_actionNotifier.pollElement());
    }

    GlobalEventManager::GlobalEventManager(GlobalEventManager &&other) : m_actionNotifier(
            std::move(other.m_actionNotifier)) {
    }

    GlobalEventManager::~GlobalEventManager() {
        m_eventLoop.remove(m_actionNotifier.pollElement());
    }

    void GlobalEventManager::notify() {
        m_actionNotifier.notify();
    }

    void GlobalEventManager::serviceEvent(const EventPollElement &event) {
        if (event.fd == m_actionNotifier.read_handle()) {
            m_actionNotifier.reset();
        } else {
            EventManager::serviceEvent(event);
        }
    }

    void GlobalEventManager::processPendingRegistrations() {
        // read from queue and add to vectors
        IOEvent *io_events[256];
        const size_t i_count = m_pendingIORegistrations.try_dequeue_bulk(io_events, 256);
        for (size_t i = 0; i < i_count; ++i) {
            IOEvent *event = io_events[i];
            m_eventLoop.add({event->fileDescriptor(), event->eventTypes()});
            m_ioHandlers.push_back(event);
            event->__setIsRegistered(true);
        }

        TimerEvent *timer_events[256];
        const size_t t_count = m_pendingTimerRegistrations.try_dequeue_bulk(timer_events, 256);
        for (size_t i = 0; i < t_count; ++i) {
            TimerEvent *event = timer_events[i];
            m_timerHandlers.push_back(event);
            event->__setIsRegistered(true);
        }

    }

    void GlobalEventManager::registerHandler(TimerEvent &eventRegistration) {
        m_pendingTimerRegistrations.try_enqueue(&eventRegistration);
        this->notify();
    }

    void GlobalEventManager::registerHandler(IOEvent &eventRegistration) {
        if (eventRegistration.fileDescriptor() <= 0) {
            ERROR_LOG("Failed to register invalid file descriptor: " << eventRegistration.fileDescriptor());
            throw EventException("Invalid file descriptor");
        }

        m_pendingIORegistrations.try_enqueue(&eventRegistration);
        this->notify();
    }

    void GlobalEventManager::unregisterHandler(TimerEvent &handler) {
//        auto it = std::find(m_pendingTimerHandlers.begin(), m_pendingTimerHandlers.end(), &handler);
//        if (it != m_pendingTimerHandlers.end()) {
//            m_pendingTimerHandlers.erase(it);
//        }
//        if (m_servicingTimers) {
//            m_pendingTimerRegistrationEvents = true;
//        } else {
//            auto it = std::find(m_timerHandlers.begin(), m_timerHandlers.end(), &handler);
//            if (it != m_timerHandlers.end()) {
//                m_timerHandlers.erase(it);
//            }
//        }

        this->notify();
    }

    void GlobalEventManager::unregisterHandler(IOEvent &handler) {
//        auto it = std::find(m_pendingHandlers.begin(), m_pendingHandlers.end(), &handler);
//        if (it != m_pendingHandlers.end()) {
//            m_pendingHandlers.erase(it);
//
//            if (handler.eventTypes() != EventType::ALL) {
//                it = std::find_if(m_pendingHandlers.begin(), m_pendingHandlers.end(), [&handler](const IOEvent *other) {
//                    return other->fileDescriptor() == handler.fileDescriptor() && (other->eventTypes() & handler.eventTypes()) == handler.eventTypes();
//                });
//                if (it == m_pendingHandlers.end()) {
//                    m_eventLoop.remove({handler.fileDescriptor(), handler.eventTypes()});
//                }
//            } else {
//                EventType eventTypes = handler.eventTypes();
//                std::for_each(m_pendingHandlers.begin(), m_pendingHandlers.end(), [&eventTypes, &handler](const IOEvent *other) {
//                    if (other->fileDescriptor() == handler.fileDescriptor()) {
//                        eventTypes = static_cast<EventType>(eventTypes & ~other->eventTypes());
//                    }
//                });
//                if (eventTypes != EventType::NONE) {
//                    m_eventLoop.remove({handler.fileDescriptor(), eventTypes});
//                }
//            }
//        }
//
//        if (m_servicingEvents) {
//            m_pendingFileDescriptorRegistrationEvents = true;
//        } else {
//            auto it = std::find(m_handlers.begin(), m_handlers.end(), &handler);
//            if (it != m_handlers.end()) {
//                m_handlers.erase(it);
//            }
//        }
        this->notify();
    }
}