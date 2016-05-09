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

#ifndef FASTPATH_SESSION_H
#define FASTPATH_SESSION_H

#include <thread>
#include <atomic>
#include <mutex>
#include <signal.h>
#include <condition_variable>

#include "fastpath/status.h"
#include "fastpath/event/GlobalEventManager.h"
#include "fastpath/utils/logger.h"

namespace fp {
    class Session {
    private:
        bool m_started;
        std::atomic_bool m_shutdown = ATOMIC_VAR_INIT(false);
        std::unique_ptr<GlobalEventManager> m_eventManager;

        std::thread m_eventLoop;

        Session() : m_started(false) {}

        const status start() {
            if (m_started) {
                return OK;
            }

            m_eventManager = std::make_unique<GlobalEventManager>();
            std::mutex mutex;
            std::condition_variable condition;

            std::unique_lock<std::mutex> lock(mutex);

            sigset_t mask;
            sigfillset(&mask);
            if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
                ERROR_LOG("Failed to block default signal delivery to event dispatch thread");
            }

            m_eventLoop = std::thread([&]() {
                {
                    std::lock_guard<std::mutex> lock_guard(mutex);
                    condition.notify_all();
                }
                while (!m_shutdown.load()) {
                    m_eventManager->waitForEvent();
                }

                m_shutdown.store(false);
                m_eventManager.reset(nullptr);
                DEBUG_LOG("Event loop exit");
            });

            condition.wait(lock);
            m_started = true;

            return OK;
        }

        const status stop() {
            if (!m_started) {
                return EVM_NOTRUNNING;
            }

            m_shutdown.store(true);
            m_eventManager->notify();
            if (m_eventLoop.joinable()) {
                m_eventLoop.join();
            }

            m_started = false;
            return OK;
        }

        static Session &instance() {
            static Session s_instance;
            return s_instance;
        }

    public:
        static status initialise() {
            return Session::instance().start();
        }

        static status destroy() {
            return Session::instance().stop();
        }

        static inline bool is_started() noexcept {
            return Session::instance().m_started;
        }

        friend class Queue;
    };
}

#endif //FASTPATH_SESSION_H
