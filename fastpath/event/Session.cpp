/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 15/06/2016

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

#include "fastpath/event/Queue.h"
#include "fastpath/Exception.h"
#include "fastpath/event/DataEvent.h"
#include "fastpath/event/TimerEvent.h"
#include "fastpath/event/SignalEvent.h"
#include "fastpath/event/MessageListener.h"
#include "fastpath/event/Subscriber.h"
#include "fastpath/utils/tfthread_utils.h"

namespace fp {
    Session::Session() : m_started(false) {}

    const status Session::start() {
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

    const status Session::stop() {
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

    bool Session::_assign_to_cpu(std::initializer_list<int> cpu_core_ids) throw(exception) {
        if (tf::thread::can_set_affinity()) {
            tf::thread::set_affinity(m_eventLoop, cpu_core_ids);
            return true;
        }
        return false;
    }

    Session &Session::instance() {
        static Session s_instance;
        return s_instance;
    }

    status Session::initialise() {
        return Session::instance().start();
    }

    status Session::destroy() {
        return Session::instance().stop();
    }
}