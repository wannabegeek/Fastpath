#ifndef tfpool_h
#define tfpool_h

#include <vector>
#include <stack>
#include <assert.h>
#include <algorithm>
#include <functional>
#include <memory>
#include <type_traits>

#include "tfspinlock.h"
#include "optimize.h"

namespace tf {

    class reusable {
    private:
        std::function<void(reusable *)> m_release_fn;

    public:
        virtual ~reusable() {}
        virtual void prepareForReuse() = 0;

        void release() {
           if (m_release_fn != nullptr) {
               m_release_fn(this);
           }
        }

        template<typename T, typename J> friend class pool;
    };

    template<class T, typename Enable = void> class pool {
        static_assert(std::is_default_constructible<T>::value, "T must be default constructable");

        size_t m_poolSize;
        std::vector<T *> m_freeAllocations;
        std::vector<T *> m_objectCache;
        mutable tf::spinlock m_lock;

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

        T *allocate() {
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

        void release(T *object) {
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

        bool is_from_pool(T *object) const {
            auto it = std::find(m_objectCache.begin(), m_objectCache.end(), object);
            return it != m_objectCache.end();
        }

        const double utilisation() const {
            m_lock.lock();
            double result = static_cast<double>(m_freeAllocations.size()) / static_cast<double>(m_poolSize);
            m_lock.unlock();
            return result;
        }
    };

    template<class T> class pool<T, typename std::enable_if<std::is_base_of<tf::reusable, T>::value>::type> {
        static_assert(std::is_default_constructible<T>::value, "T must be default constructable");

        size_t m_poolSize;
        std::vector<T *> m_freeAllocations;
        std::vector<T *> m_objectCache;
        mutable tf::spinlock m_lock;

        std::function<void(reusable *)> m_release_function;

        std::function<void(T *)> m_deleter;

    public:
        using unique_ptr_type = std::unique_ptr<T, decltype(m_deleter)>;
        using shared_ptr_type = std::shared_ptr<T>;

        pool(size_t poolSize = 100) : m_release_function(std::bind(&pool<T>::release, this, std::placeholders::_1)) {
            m_deleter = [](T *p){
                p->release();
            };
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
            INFO_LOG("Removing Queue " << m_freeAllocations.size() << " vs " << m_objectCache.size());
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

        std::function<void(T *)> pool_release_fn() const noexcept {
            return m_deleter;
        }

        unique_ptr_type allocate_unique_ptr() {
            return unique_ptr_type(allocate(), m_deleter);
        }

        shared_ptr_type allocate_shared_ptr() {
            return shared_ptr_type(allocate(), m_deleter);
        }

        T *allocate() {
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

            INFO_LOG("Allocating " << object << " from the pool");
            return object;
        }

        void release(reusable *object) {
            if (object != nullptr) {
                INFO_LOG("Releasing " << object << " back to the pool");
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

        bool is_from_pool(reusable *object) const {
            auto it = std::find(m_objectCache.begin(), m_objectCache.end(), object);
            return it != m_objectCache.end();
        }


        const double utilisation() const {
            m_lock.lock();
            double result = static_cast<double>(m_freeAllocations.size()) / static_cast<double>(m_poolSize);
            m_lock.unlock();
            return result;
        }
    };
}

#endif
