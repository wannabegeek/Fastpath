//
// Created by Tom Fewster on 07/03/2016.
//

#include "InlineEventManager.h"
#include "IOEvent.h"
#include "TimerEvent.h"


namespace DCF {

    InlineEventManager::InlineEventManager() : m_pendingFileDescriptorRegistrationEvents(false), m_pendingTimerRegistrationEvents(false) {
    }

    InlineEventManager::InlineEventManager(InlineEventManager &&other) : EventManager(std::move(other)), m_pendingFileDescriptorRegistrationEvents(false), m_pendingTimerRegistrationEvents(false) {
    }

    InlineEventManager::~InlineEventManager() {
    }


    void InlineEventManager::processPendingRegistrations() {
        if (m_pendingTimerRegistrationEvents) {
            m_timerHandlers = m_pendingTimerHandlers;
            m_pendingTimerRegistrationEvents = false;
        }
        if (m_pendingFileDescriptorRegistrationEvents) {
            m_ioHandlers = m_pendingHandlers;
            m_pendingFileDescriptorRegistrationEvents = false;
        }
    }

    void InlineEventManager::registerHandler(TimerEvent &eventRegistration) {
        if (m_servicingTimers) {
            m_pendingTimerRegistrationEvents = true;
            m_pendingTimerHandlers.push_back(&eventRegistration);
        } else {
            m_timerHandlers.push_back(&eventRegistration);
            m_pendingTimerHandlers.push_back(&eventRegistration);
        }
        eventRegistration.__setIsRegistered(true);
    }

    void InlineEventManager::registerHandler(IOEvent &eventRegistration) {
        if (eventRegistration.fileDescriptor() <= 0) {
            ERROR_LOG("Failed to register invalid file descriptor: " << eventRegistration.fileDescriptor());
            throw EventException("Invalid file descriptor");
        }

        if (m_servicingEvents) {
            m_pendingHandlers.push_back(&eventRegistration);
            m_pendingFileDescriptorRegistrationEvents = true;
        } else {
            m_ioHandlers.push_back(&eventRegistration);
            m_pendingHandlers.push_back(&eventRegistration);
        }
        m_eventLoop.add({eventRegistration.fileDescriptor(), eventRegistration.eventTypes()});
        eventRegistration.__setIsRegistered(true);
    }

    void InlineEventManager::unregisterHandler(TimerEvent &handler) {
        auto it = std::find(m_pendingTimerHandlers.begin(), m_pendingTimerHandlers.end(), &handler);
        if (it != m_pendingTimerHandlers.end()) {
            m_pendingTimerHandlers.erase(it);
        }
        if (m_servicingTimers) {
            m_pendingTimerRegistrationEvents = true;
        } else {
            auto it = std::find(m_timerHandlers.begin(), m_timerHandlers.end(), &handler);
            if (it != m_timerHandlers.end()) {
                m_timerHandlers.erase(it);
            }
        }

        handler.__setIsRegistered(false);
    }

    void InlineEventManager::unregisterHandler(IOEvent &handler) {
        auto it = std::find(m_pendingHandlers.begin(), m_pendingHandlers.end(), &handler);
        if (it != m_pendingHandlers.end()) {
            m_pendingHandlers.erase(it);

            if (handler.eventTypes() != EventType::ALL) {
                it = std::find_if(m_pendingHandlers.begin(), m_pendingHandlers.end(), [&handler](const IOEvent *other) {
                    return other->fileDescriptor() == handler.fileDescriptor() && (other->eventTypes() & handler.eventTypes()) == handler.eventTypes();
                });
                if (it == m_pendingHandlers.end()) {
                    m_eventLoop.remove({handler.fileDescriptor(), handler.eventTypes()});
                }
            } else {
                EventType eventTypes = handler.eventTypes();
                std::for_each(m_pendingHandlers.begin(), m_pendingHandlers.end(), [&eventTypes, &handler](const IOEvent *other) {
                    if (other->fileDescriptor() == handler.fileDescriptor()) {
                        eventTypes = static_cast<EventType>(eventTypes & ~other->eventTypes());
                    }
                });
                if (eventTypes != EventType::NONE) {
                    m_eventLoop.remove({handler.fileDescriptor(), eventTypes});
                }
            }
        }

        if (m_servicingEvents) {
            m_pendingFileDescriptorRegistrationEvents = true;
        } else {
            auto it = std::find(m_ioHandlers.begin(), m_ioHandlers.end(), &handler);
            if (it != m_ioHandlers.end()) {
                m_ioHandlers.erase(it);
            }
        }

        handler.__setIsRegistered(false);
    }
}