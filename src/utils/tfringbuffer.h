//
// Created by fewstert on 10/03/16.
//

#ifndef TFDCF_TFRING_BUFFER_H
#define TFDCF_TFRING_BUFFER_H

#include <atomic>
#include <cstddef>
#include <memory>
#include "optimize.h"

#ifndef CACHELINE_SIZE
#   define CACHELINE_SIZE 64
#endif

namespace tf {
    template<typename T, size_t SIZE = 1024, typename Allocator = std::allocator<T>>
    class ringbuffer {
        using storage_alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<T>;
        using storage_traits = typename std::allocator_traits<Allocator>::template rebind_traits<T>;

        storage_alloc m_allocator;

        T *m_buffer;
        alignas(CACHELINE_SIZE) std::atomic<size_t> m_head;
        alignas(CACHELINE_SIZE) std::atomic<size_t> m_tail;

        size_t next(size_t current) {
            return (current + 1) % SIZE;
        }

    public:
        static constexpr size_t max_size = SIZE;

        ringbuffer() : m_head(0), m_tail(0) {
            m_buffer = storage_traits::allocate(m_allocator, SIZE);
        }

        virtual ~ringbuffer() {
            storage_traits::deallocate(m_allocator, m_buffer, SIZE);
        }

        virtual bool push(const T &&object) {
            size_t head = m_head.load(std::memory_order_relaxed);
            size_t nextHead = next(head);
            if (unlikely(nextHead == m_tail.load(std::memory_order_acquire))) {
                return false;
            }
            m_buffer[head] = std::move(object);
            m_head.store(nextHead, std::memory_order_release);

            return true;
        }

        virtual bool push(const T &object) {
            size_t head = m_head.load(std::memory_order_relaxed);
            size_t nextHead = next(head);
            if (unlikely(nextHead == m_tail.load(std::memory_order_acquire))) {
                return false;
            }
            m_buffer[head] = object;
            m_head.store(nextHead, std::memory_order_release);

            return true;
        }

        bool pop(T &object) {
            size_t tail = m_tail.load(std::memory_order_relaxed);
            if (likely(tail == m_head.load(std::memory_order_acquire))) {
                return false;
            }

            object = std::move(m_buffer[tail]);
            m_buffer[tail].~T();
            m_tail.store(next(tail), std::memory_order_release);
            return true;
        }

        const size_t size() const {
            const size_t head = m_head.load(std::memory_order_relaxed);
            const size_t tail = m_tail.load(std::memory_order_relaxed);
            return head >= tail ? head - tail : head + SIZE - tail;
        }
    };
}

#endif //TFDCF_TFRING_BUFFER_H
