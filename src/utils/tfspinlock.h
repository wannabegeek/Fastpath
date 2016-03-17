//
//  TFSpinLock.h
//  CeloxicaTest
//
//  Created by Tom Fewster on 25/04/2013.
//

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
