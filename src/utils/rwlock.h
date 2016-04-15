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

#ifndef TFDCF_RWLOCK_H
#define TFDCF_RWLOCK_H

#include <atomic>
#include <thread>
#include "optimize.h"

namespace tf {
    class rwlock {
    private:
        enum : int32_t { READER = 4, UPGRADED = 2, WRITER = 1 };
        std::atomic<int32_t> m_bits;
    public:
        constexpr rwlock() : m_bits(0) {}

        rwlock(const rwlock&) = delete;
        rwlock &operator=(rwlock const &) = delete;

        void lock() {
            int count = 0;
            while (unlikely(!try_lock())) {
                if (++count > 1000) {
                    std::this_thread::yield();
                }
            }
        }

        void unlock() {
            static_assert(READER > WRITER + UPGRADED, "wrong bits!");
            m_bits.fetch_and(~(WRITER | UPGRADED), std::memory_order_release);
        }

        // SharedLockable Concept
        void lock_shared() {
            int count = 0;
            while (unlikely(!try_lock_shared())) {
                if (++count > 1000) {
                    std::this_thread::yield();
                }
            }
        }

        void unlock_shared() {
            m_bits.fetch_add(-READER, std::memory_order_release);
        }

        // Downgrade the lock from writer status to reader status.
        void unlock_and_lock_shared() {
            m_bits.fetch_add(READER, std::memory_order_acquire);
            unlock();
        }

        // UpgradeLockable Concept
        void lock_upgrade() {
            int count = 0;
            while (!try_lock_upgrade()) {
                if (++count > 1000) {
                    std::this_thread::yield();
                }
            }
        }

        void unlock_upgrade() {
            m_bits.fetch_add(-UPGRADED, std::memory_order_acq_rel);
        }

        // unlock upgrade and try to acquire write lock
        void unlock_upgrade_and_lock() {
            int64_t count = 0;
            while (!try_unlock_upgrade_and_lock()) {
                if (++count > 1000) {
                    std::this_thread::yield();
                }
            }
        }

        // unlock upgrade and read lock atomically
        void unlock_upgrade_and_lock_shared() {
            m_bits.fetch_add(READER - UPGRADED, std::memory_order_acq_rel);
        }

        // write unlock and upgrade lock atomically
        void unlock_and_lock_upgrade() {
            // need to do it in two steps here -- as the UPGRADED bit might be OR-ed at
            // the same time when other threads are trying do try_lock_upgrade().
            m_bits.fetch_or(UPGRADED, std::memory_order_acquire);
            m_bits.fetch_add(-WRITER, std::memory_order_release);
        }


        // Attempt to acquire writer permission. Return false if we didn't get it.
        bool try_lock() {
            int32_t expect = 0;
            return m_bits.compare_exchange_strong(expect, WRITER,
                                                 std::memory_order_acq_rel);
        }

        // Try to get reader permission on the lock. This can fail if we
        // find out someone is a writer or upgrader.
        // Setting the UPGRADED bit would allow a writer-to-be to indicate
        // its intention to write and block any new readers while waiting
        // for existing readers to finish and release their read locks. This
        // helps avoid starving writers (promoted from upgraders).
        bool try_lock_shared() {
            // fetch_add is considerably (100%) faster than compare_exchange,
            // so here we are optimizing for the common (lock success) case.
            int32_t value = m_bits.fetch_add(READER, std::memory_order_acquire);
            if (unlikely(value & (WRITER|UPGRADED))) {
                m_bits.fetch_add(-READER, std::memory_order_release);
                return false;
            }
            return true;
        }

        // try to unlock upgrade and write lock atomically
        bool try_unlock_upgrade_and_lock() {
            int32_t expect = UPGRADED;
            return m_bits.compare_exchange_strong(expect, WRITER,
                                                 std::memory_order_acq_rel);
        }

        // try to acquire an upgradable lock.
        bool try_lock_upgrade() {
            int32_t value = m_bits.fetch_or(UPGRADED, std::memory_order_acquire);

            // Note: when failed, we cannot flip the UPGRADED bit back,
            // as in this case there is either another upgrade lock or a write lock.
            // If it's a write lock, the bit will get cleared up when that lock's done
            // with unlock().
            return ((value & (UPGRADED | WRITER)) == 0);
        }

        // mainly for debugging purposes.
        int32_t bits() const {
            return m_bits.load(std::memory_order_acquire);
        }

        class read_holder;
        class upgraded_holder;
        class write_holder;

        class read_holder {
        private:
            friend class upgraded_holder;
            friend class write_holder;
            rwlock *m_lock;

        public:
            explicit read_holder(rwlock *lock = nullptr) : m_lock(lock) {
                if (m_lock) {
                    m_lock->lock_shared();
                }
            }

            explicit read_holder(rwlock &lock) : m_lock(&lock) {
                m_lock->lock_shared();
            }

            read_holder(read_holder &&other) noexcept : m_lock(other.m_lock) {
                other.m_lock = nullptr;
            }

            // down-grade
            explicit read_holder(upgraded_holder &&upgraded) : m_lock(upgraded.m_lock) {
                upgraded.m_lock = nullptr;
                if (m_lock) {
                    m_lock->unlock_upgrade_and_lock_shared();
                }
            }

            explicit read_holder(write_holder &&writer) : m_lock(writer.m_lock) {
                writer.m_lock = nullptr;
                if (m_lock) {
                    m_lock->unlock_and_lock_shared();
                }
            }

            read_holder &operator=(read_holder &&other) {
                std::swap(m_lock, other.m_lock);
                return *this;
            }

            read_holder(const read_holder &other) = delete;
            read_holder& operator=(const read_holder &other) = delete;

            ~read_holder() {
                if (m_lock) {
                    m_lock->unlock_shared();
                }
            }

            void reset(rwlock *lock = nullptr) {
                if (lock == m_lock) {
                    return;
                }
                if (m_lock) {
                    m_lock->unlock_shared();
                }
                m_lock = lock;
                if (m_lock) {
                    m_lock->lock_shared();
                }
            }

            void swap(read_holder *other) {
                std::swap(m_lock, other->m_lock);
            }
        };

        class upgraded_holder {
        private:
            friend class write_holder;
            friend class read_holder;
            rwlock *m_lock;

        public:
            explicit upgraded_holder(rwlock *lock = nullptr) : m_lock(lock) {
                if (m_lock) {
                    m_lock->lock_upgrade();
                }
            }

            explicit upgraded_holder(rwlock &lock) : m_lock(&lock) {
                m_lock->lock_upgrade();
            }

            explicit upgraded_holder(write_holder &&writer) {
                m_lock = writer.m_lock;
                writer.m_lock = nullptr;
                if (m_lock) {
                    m_lock->unlock_and_lock_upgrade();
                }
            }

            upgraded_holder(upgraded_holder &&other) noexcept : m_lock(other.m_lock) {
                other.m_lock = nullptr;
            }

            upgraded_holder &operator =(upgraded_holder &&other) {
                std::swap(m_lock, other.m_lock);
                return *this;
            }

            upgraded_holder(const upgraded_holder &other) = delete;
            upgraded_holder& operator=(const upgraded_holder &other) = delete;

            ~upgraded_holder() {
                if (m_lock) {
                    m_lock->unlock_upgrade();
                }
            }

            void reset(rwlock *lock = nullptr) {
                if (lock == m_lock) {
                    return;
                }
                if (m_lock) {
                    m_lock->unlock_upgrade();
                }
                m_lock = lock;
                if (m_lock) {
                    m_lock->lock_upgrade();
                }
            }

            void swap(upgraded_holder *other) {
                std::swap(m_lock, other->m_lock);
            }
        };

        class write_holder {
        private:
            friend class read_holder;
            friend class upgraded_holder;
            rwlock *m_lock;

        public:
            explicit write_holder(rwlock *lock = nullptr) : m_lock(lock) {
                if (m_lock) {
                    m_lock->lock();
                }
            }

            explicit write_holder(rwlock &lock) : m_lock(&lock) {
                m_lock->lock();
            }

            // promoted from an upgrade lock holder
            explicit write_holder(upgraded_holder &&upgraded) {
                m_lock = upgraded.m_lock;
                upgraded.m_lock = nullptr;
                if (m_lock) {
                    m_lock->unlock_upgrade_and_lock();
                }
            }

            write_holder(write_holder &&other) noexcept : m_lock(other.m_lock) {
                other.m_lock = nullptr;
            }

            write_holder& operator =(write_holder&& other) {
                std::swap(m_lock, other.m_lock);
                return *this;
            }

            write_holder(const write_holder &other) = delete;
            write_holder& operator =(const write_holder &other) = delete;

            ~write_holder () {
                if (m_lock) {
                    m_lock->unlock();
                }
            }

            void reset(rwlock *lock = nullptr) {
                if (lock == m_lock) {
                    return;
                }
                if (m_lock) {
                    m_lock->unlock();
                }
                m_lock = lock;
                if (m_lock) {
                    m_lock->lock();
                }
            }

            void swap(write_holder *other) {
                std::swap(m_lock, other->m_lock);
            }
        };

        // Synchronized<> adaptors
        friend void acquireRead(rwlock &l) { return l.lock_shared(); }
        friend void acquireReadWrite(rwlock &l) { return l.lock(); }
        friend void releaseRead(rwlock &l) { return l.unlock_shared(); }
        friend void releaseReadWrite(rwlock &l) { return l.unlock(); }

    };
}

#endif //TFDCF_RWLOCK_H
