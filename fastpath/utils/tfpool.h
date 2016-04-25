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

#ifndef tfpool_h
#define tfpool_h

#include <vector>
#include <stack>
#include <assert.h>
#include <algorithm>
#include <functional>
#include <memory>
#include <type_traits>

#include "fastpath/utils/optimize.h"

namespace tf {

    class reusable {
    private:
        std::function<void(reusable *)> m_release_fn;

    public:
        virtual ~reusable() {}
        virtual void prepareForReuse() = 0;

        inline void release() noexcept {
           if (m_release_fn != nullptr) {
               m_release_fn(this);
           }
        }

        template<typename T, typename L, typename J> friend class pool;
    };

    template<class T, typename L, typename Enable = void> class pool {
        static_assert(std::is_default_constructible<T>::value, "T must be default constructable");
        static_assert(std::is_nothrow_constructible<T>::value, "T must be nothrow constructable");

        size_t m_poolSize;
        std::vector<T *> m_freeAllocations;
        std::vector<T *> m_objectCache;
        mutable L m_lock;

    public:
        pool(size_t poolSize = 256) {
            m_poolSize = poolSize;
            m_freeAllocations.reserve(m_poolSize);
            m_objectCache.reserve(m_poolSize);
            for (size_t i = 0; i < m_poolSize; i++) {
                m_objectCache.emplace_back(new T());
            }
            std::copy(m_objectCache.begin(), m_objectCache.end(), std::back_inserter(m_freeAllocations));
        }

        virtual ~pool() {
            m_lock.lock();
            assert(m_freeAllocations.size() == m_objectCache.size()); // We still have objects allocated out whilst trying to destruct our pool
            std::for_each(m_objectCache.begin(), m_objectCache.end(), [&](T *obj) {
                delete obj;
            });
        }

        // delete the copy/move/assignment
        pool(pool &&other) = delete;
        pool(const pool &) = delete;
        pool &operator=(const pool &) = delete;

        inline T *allocate() noexcept {
            m_lock.lock();
            if (unlikely(m_freeAllocations.empty())) {
                size_t newPoolSize = m_poolSize * 2;
                m_freeAllocations.reserve(newPoolSize);
                m_objectCache.reserve(newPoolSize);
                for (size_t i = m_poolSize; i < newPoolSize; i++) {
                    m_objectCache.emplace_back(new T());
                }
                auto it = m_objectCache.begin();
                std::advance(it, m_poolSize);
                std::copy(it, m_objectCache.end(), std::back_inserter(m_freeAllocations));
                m_poolSize = newPoolSize;
            }

            T *object = m_freeAllocations.back();
            m_freeAllocations.pop_back();
            m_lock.unlock();
            assert(object != nullptr);

            return object;
        }

        inline void release(T *object) noexcept {
            if (object != nullptr) {
                m_lock.lock();
#ifdef CHECK_DOUBLE_FREE
                auto it = std::find(m_freeAllocations.begin(), m_freeAllocations.end(), object);
                assert(it == m_freeAllocations.end());
#endif
                m_freeAllocations.push_back(object);
                m_lock.unlock();
            }
        }

        bool is_from_pool(T *object) const noexcept {
            auto it = std::find(m_objectCache.begin(), m_objectCache.end(), object);
            return it != m_objectCache.end();
        }

        const double utilisation() const noexcept {
            m_lock.lock();
            double result = static_cast<double>(m_freeAllocations.size()) / static_cast<double>(m_poolSize);
            m_lock.unlock();
            return result;
        }
    };

    template<class T, typename L> class pool<T, L, typename std::enable_if<std::is_base_of<tf::reusable, T>::value>::type> {
        static_assert(std::is_default_constructible<T>::value, "T must be default constructable");
        static_assert(std::is_nothrow_constructible<T>::value, "T must be nothrow constructable");

        size_t m_poolSize;
        std::vector<T *> m_freeAllocations;
        std::vector<T *> m_objectCache;
        mutable L m_lock;

        std::function<void(reusable *)> m_release_function;

        std::function<void(T *)> m_deleter;

    public:
        using unique_ptr_type = std::unique_ptr<T, decltype(m_deleter)>;
        using shared_ptr_type = std::shared_ptr<T>;

        pool(size_t poolSize = 100) : m_release_function(std::bind(&pool<T, L>::release, this, std::placeholders::_1)) {
            m_poolSize = poolSize;
            m_freeAllocations.reserve(m_poolSize);
            m_objectCache.reserve(m_poolSize);
            for (size_t i = 0; i < m_poolSize; i++) {
                T *t = new T();
                t->m_release_fn = m_release_function;
                m_objectCache.emplace_back(t);
            }
            std::copy(m_objectCache.begin(), m_objectCache.end(), std::back_inserter(m_freeAllocations));
        }

        virtual ~pool() {
            m_lock.lock();
            assert(m_freeAllocations.size() == m_objectCache.size()); // We still have objects allocated out whilst trying to destruct our pool
            std::for_each(m_objectCache.begin(), m_objectCache.end(), [&](T *obj) {
                delete obj;
            });
            m_lock.unlock();
        }

        // delete the copy/move/assignment
        pool(pool &&other) = delete;
        pool(const pool &) = delete;
        pool &operator=(const pool &) = delete;

        inline std::function<void(T *)> pool_release_fn() const noexcept {
            return m_deleter;
        }

        inline unique_ptr_type allocate_unique_ptr() noexcept {
            return unique_ptr_type(allocate(), m_release_function);
        }

        inline shared_ptr_type allocate_shared_ptr() noexcept {
            return shared_ptr_type(allocate(), m_release_function);
        }

        inline T *allocate() noexcept {
            m_lock.lock();
            if (unlikely(m_freeAllocations.empty())) {
                size_t newPoolSize = m_poolSize * 2;
                m_freeAllocations.reserve(newPoolSize);
                m_objectCache.reserve(newPoolSize);
                for (size_t i = m_poolSize; i < newPoolSize; i++) {
                    T *t = new T();
                    t->m_release_fn = m_release_function;
                    m_objectCache.emplace_back(t);
                }
                auto it = m_objectCache.begin();
                std::advance(it, m_poolSize);
                std::copy(it, m_objectCache.end(), std::back_inserter(m_freeAllocations));
                m_poolSize = newPoolSize;
            }

            T *object = m_freeAllocations.back();
            m_freeAllocations.pop_back();
            m_lock.unlock();
            assert(object != nullptr);
            object->prepareForReuse();

            return object;
        }

        inline void release(reusable *object) noexcept {
            if (object != nullptr) {
                m_lock.lock();
#ifdef CHECK_DOUBLE_FREE
                auto it = std::find(m_freeAllocations.begin(), m_freeAllocations.end(), object);
                assert(it == m_freeAllocations.end());
#endif
                assert(m_freeAllocations.size() < m_poolSize);
                m_freeAllocations.push_back(static_cast<T *>(object));
                m_lock.unlock();
            }
        }

        bool is_from_pool(reusable *object) const noexcept {
            auto it = std::find(m_objectCache.begin(), m_objectCache.end(), object);
            return it != m_objectCache.end();
        }

        const double utilisation() const noexcept {
            m_lock.lock();
            double result = static_cast<double>(m_freeAllocations.size()) / static_cast<double>(m_poolSize);
            m_lock.unlock();
            return result;
        }
    };
}

#endif
