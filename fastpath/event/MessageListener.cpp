/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/
#include "fastpath/event/MessageListener.h"
#include "fastpath/utils/logger.h"
#include "fastpath/event/IOEvent.h"
#include "fastpath/event/Queue.h"
#include "fastpath/transport/TCPTransport.h"

namespace fp {

    TransportContainer::TransportContainer(Transport *t, std::unique_ptr<TransportIOEvent> &&e) : transport(t), event(std::move(e)) {}

    void MessageListener::subscribe(Transport *transport, const char *subject) noexcept {
        MutableMessage msg;
        msg.setSubject("_FP.REGISTER.OBSERVER");
        msg.addDataField("subject", subject);
        msg.addScalarField("id", reinterpret_cast<uint64_t>(this));
        transport->sendMessage(msg);
    }

    void MessageListener::unsubscribe(Transport *transport, const char *subject) noexcept {
        MutableMessage msg;
        msg.setSubject("_FP.UNREGISTER.OBSERVER");
        msg.addDataField("subject", subject);
        msg.addScalarField("id", reinterpret_cast<uint64_t>(this));
        transport->sendMessage(msg);
    }

    const bool MessageListener::registerTransport(Transport *transport, EventManager *eventManager) {
        if (transport->m_eventManager == nullptr) {
            // lock
            if (transport->m_eventManager == nullptr) {
                transport->m_eventManager = eventManager;
                std::unique_ptr<TransportContainer> container = std::make_unique<TransportContainer>(transport, transport->createReceiverEvent(std::bind(&MessageListener::handleMessage, this, std::placeholders::_1, std::placeholders::_2)));
                if (container->event) {
                    eventManager->registerHandler(container->event.get());
                }
                m_transportConnections.push_back(std::move(container));

                auto &transport_container = m_transportConnections.back();
                transport->setNotificationHandler([&, this, transport, eventManager](Transport::notification_type type, const char *reason) {
                    switch (type) {
                        case Transport::CONNECTED: {
                            DEBUG_LOG("We have just been connected to fprouter, need to re-subscribe");
                            transport_container->event = transport->createReceiverEvent(std::bind(&MessageListener::handleMessage, this, std::placeholders::_1, std::placeholders::_2));
                            if (transport_container->event) {
                                eventManager->registerHandler(transport_container->event.get());
                            }
                            auto it = this->m_observers.find(transport);
                            if (it != this->m_observers.end()) {
                                const ObserversType &subscribers = it->second;
                                for (const auto &message_event : subscribers) {
                                    this->subscribe(transport, message_event->subscriber()->subject());
                                }
                            }
                            break;
                        }
                        case Transport::DISCONNECTED:
                            DEBUG_LOG("We have just been disconnected from fprouter");
                            if (transport_container->event) {
                                if (eventManager != nullptr) {
                                    eventManager->unregisterHandler(transport_container->event.get());
                                }
                                transport_container->event.reset();
                            }
                            break;
                        default:
                            // not handling CORRUPT_MESSAGE etc. here
                            break;
                    }
                });
            }
            // unlock
            return true;
        } else if (transport->m_eventManager != eventManager) {
            ERROR_LOG("Transport cannot be associated with a global queue and an inline dispatch queue");
        } else {
            return true;
        }

        return false;
    }


    void MessageListener::handleMessage(const Transport *transport, Transport::MessageType &message) {
        auto it = m_observers.find(transport);
        if (it != m_observers.end()) {
            subject<> subject(message->subject());
            ObserversType &observers = it->second;
            std::for_each(observers.begin(), observers.end(), [&](auto &messageEvent) noexcept {
                if (messageEvent->subscriber()->is_interested(subject)) {
                    messageEvent->__notify(message);
                }
            });
        }
    }

    status MessageListener::addObserver(Queue *queue, const Subscriber &subscriber, EventManager *eventManager) noexcept {

        if (this->registerTransport(subscriber.transport(), eventManager)) {
            auto it = m_observers.find(subscriber.transport());
            if (it == m_observers.end()) {
                ObserversType subscribers;
                subscribers.emplace_back(std::make_unique<MessageEvent>(queue, &subscriber));
                m_observers.emplace(subscriber.transport(), std::move(subscribers));
            } else {
                ObserversType &subscribers = it->second;
                subscribers.emplace_back(std::make_unique<MessageEvent>(queue, &subscriber));
            }
            if (subscriber.transport()->valid()) {
                this->subscribe(subscriber.transport(), subscriber.subject());
            }
        } else {
            return INVALID_TRANSPORT_STATE;
        }

        return OK;

    }

    status MessageListener::removeObserver(Queue *queue, const Subscriber &subscriber) noexcept {
        auto it = m_observers.find(subscriber.transport());
        if (it != m_observers.end()) {
            ObserversType &subscribers = it->second;
            auto it2 = std::find_if(subscribers.begin(), subscribers.end(), [&subscriber] (const auto &s) {
                return s->subscriber() == &subscriber;
            });
            if (it2 != subscribers.end()) {
                this->unsubscribe(subscriber.transport(), subscriber.subject());
                subscribers.erase(it2);
                return OK;
            }
        }

        return CANNOT_DESTROY;
    }
}