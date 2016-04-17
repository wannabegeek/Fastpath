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

#ifndef tfspinlock_h
#define tfspinlock_h

#include <iostream>
#include <atomic>

namespace tf {

	class spinlock {
		std::atomic_flag m_locked = ATOMIC_FLAG_INIT;

	public:
		inline void lock() {
            while (m_locked.test_and_set(std::memory_order_acquire));
		}

		inline void unlock() {
            m_locked.clear(std::memory_order_release);
		}
	};

	class spinlock_auto {
		spinlock &m_lock;
	public:
		explicit spinlock_auto(spinlock &lock) : m_lock(lock) {
			m_lock.lock();
		}

		~spinlock_auto() {
			m_lock.unlock();
		}
	};
}

#endif
