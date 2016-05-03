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

#ifndef FASTPATH_BACKOFFSTRATEGY_H
#define FASTPATH_BACKOFFSTRATEGY_H

#include <chrono>
#include <thread>

namespace fp {
    class BackoffStrategy {
    private:
        static constexpr std::chrono::milliseconds min_retry_interval() { return std::chrono::milliseconds(1); }
        static constexpr std::chrono::milliseconds max_retry_interval() { return std::chrono::seconds(5); }

        std::chrono::milliseconds m_current;
    public:
        BackoffStrategy() : m_current(min_retry_interval()) {}

        void backoff() noexcept {
            std::this_thread::sleep_for(m_current);
            m_current = std::min(m_current * 2, max_retry_interval());
        }
    };
}
#endif //FASTPATH_BACKOFFSTRATEGY_H
