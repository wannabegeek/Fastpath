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

#ifndef tfpool_new_h
#define tfpool_new_h

#include <vector>
#include <stack>
#include <assert.h>
#include <algorithm>
#include <functional>
#include <memory>
#include <type_traits>

#include "fastpath/utils/tfspinlock.h"
#include "fastpath/utils/optimize.h"
#include "fastpath/utils/logger.h"

namespace tf {
    namespace experimental {

        template <typename T> struct global_pool {

            static constexpr std::size_t replenish_count = 100000;
            static constexpr std::size_t obtain_count = 1000;

            std::size_t m_poolSize;
            std::vector<T *> m_freeAllocations;
            std::vector<T *> m_objectCache;
            mutable tf::spinlock m_lock;

            global_pool(size_t poolSize = 256) noexcept : m_poolSize(poolSize) {
                m_freeAllocations.reserve(m_poolSize);
                m_objectCache.reserve(m_poolSize);
                for (size_t i = 0; i < m_poolSize; i++) {
                    m_objectCache.emplace_back(new T());
                }
                std::copy(m_objectCache.begin(), m_objectCache.end(), std::back_inserter(m_freeAllocations));
            }

            // delete the copy/move/assignment
            global_pool(global_pool &&other) = delete;
            global_pool(const global_pool &) = delete;
            global_pool &operator=(const global_pool &) = delete;

            virtual ~global_pool() noexcept {
                m_lock.lock();
                assert(m_freeAllocations.size() == m_objectCache.size()); // We still have objects allocated out whilst trying to destruct our pool
                std::for_each(m_objectCache.begin(), m_objectCache.end(), [&](T *obj) {
                    delete obj;
                });
            }

            void replenish(const typename decltype(m_freeAllocations)::const_iterator &begin, const typename decltype(m_freeAllocations)::const_iterator &end) noexcept {
                m_lock.lock();
                std::move(begin, end, std::back_inserter(m_freeAllocations));
                m_lock.unlock();
            }

            bool obtain(std::vector<T *> &objects) noexcept {
                if (tf::unlikely(m_freeAllocations.empty())) {
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

                std::move(m_freeAllocations.begin(), m_freeAllocations.begin() + obtain_count, std::back_inserter(objects));
                return true;
            }

        };

        template<class T>
        class pool {
            static_assert(std::is_default_constructible<T>::value, "T must be default constructable");

            static __thread std::vector<T *> *m_freeAllocations;

            using global_pool_type = global_pool<T>;
            global_pool_type m_global_pool;

        public:
            pool() noexcept : m_global_pool(10000) {
                if (m_freeAllocations == nullptr) {
                    m_freeAllocations = new std::vector<T *>;
                }
            }

            // delete the copy/move/assignment
            pool(pool &&other) = delete;
            pool(const pool &) = delete;
            pool &operator=(const pool &) = delete;

            T *allocate() noexcept {
                if (tf::unlikely(m_freeAllocations->empty())) {
                    INFO_LOG("Get from global pool");
                    m_global_pool.obtain(*m_freeAllocations);
                }

                T *object = m_freeAllocations->back();
                m_freeAllocations->pop_back();
                assert(object != nullptr);

                return object;
            }

            void release(T *object) noexcept {
                if (tf::unlikely(object != nullptr)) {
#ifdef CHECK_DOUBLE_FREE
                    auto it = std::find(m_freeAllocations->begin(), m_freeAllocations->end(), object);
                    assert(it == m_freeAllocations->end());
#endif
                    m_freeAllocations->push_back(object);

                    if (m_freeAllocations->size() >= global_pool_type::replenish_count) {
                        INFO_LOG("Back to global pool");
                        m_global_pool.replenish(m_freeAllocations->cbegin(), m_freeAllocations->cbegin() + (global_pool_type::replenish_count - global_pool_type::obtain_count));
                    }
                }
            }

//            bool is_from_pool(T *object) const {
//                auto it = std::find(m_objectCache.begin(), m_objectCache.end(), object);
//                return it != m_objectCache.end();
//            }

//            const double utilisation() const {
//                m_lock.lock();
//                double result = static_cast<double>(m_freeAllocations.size()) / static_cast<double>(m_poolSize);
//                m_lock.unlock();
//                return result;
//            }
        };

        template<class T> __thread std::vector<T *> *pool<T>::m_freeAllocations;
    }
}

#endif
