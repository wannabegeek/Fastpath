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

        Session();
        const status start();
        const status stop();
        bool _assign_to_cpu(std::initializer_list<int> cpu_core_ids) throw(exception);

        static Session &instance();

    public:
        static status initialise();
        static status destroy();

        static inline bool is_started() noexcept {
            return Session::instance().m_started;
        }

        static bool assign_to_cpu(std::initializer_list<int> cpu_core_ids) throw(exception) {
            return Session::instance()._assign_to_cpu(cpu_core_ids);
        }

        friend class Queue;
    };
}

#endif //FASTPATH_SESSION_H
