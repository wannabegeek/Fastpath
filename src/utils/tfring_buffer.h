//
// Created by fewstert on 10/03/16.
//

#ifndef TFDCF_TFRING_BUFFER_H
#define TFDCF_TFRING_BUFFER_H

#include <atomic>
#include <cstddef>
#include <array>

namespace tf {
    template<typename T, size_t SIZE = 1024>
    class ring_buffer {
        std::array<T, SIZE> m_buffer;
        std::atomic<size_t> m_head;
        std::atomic<size_t> m_tail;

        size_t next(size_t current) {
            return (current + 1) % SIZE;
        }

    public:
        constexpr size_t size = SIZE;

        ring_buffer() : m_head(0), m_tail(0) {
        }

        virtual ~ring_buffer() { }

        bool push(const T &&object) {
            size_t head = m_head.load(std::memory_order_relaxed);
            size_t nextHead = next(head);
            if (nextHead == m_tail.load(std::memory_order_acquire)) {
                return false;
            }
            m_buffer[head] = std::move(object);
            m_head.store(nextHead, std::memory_order_release);

            return true;
        }

        bool push(const T &object) {
            size_t head = m_head.load(std::memory_order_relaxed);
            size_t nextHead = next(head);
            if (nextHead == m_tail.load(std::memory_order_acquire)) {
                return false;
            }
            m_buffer[head] = object;
            m_head.store(nextHead, std::memory_order_release);

            return true;
        }

        bool pop(T &&object) {
            size_t tail = m_tail.load(std::memory_order_relaxed);
            if (tail == m_head.load(std::memory_order_acquire)) {
                return false;
            }

            object = std::move(m_buffer[tail]);
            m_tail.store(next(tail), std::memory_order_release);
            return true;
        }

        bool pop(T &&object) {
            size_t tail = m_tail.load(std::memory_order_relaxed);
            if (tail == m_head.load(std::memory_order_acquire)) {
                return false;
            }

            object = m_buffer[tail];
            m_tail.store(next(tail), std::memory_order_release);
            return true;
        }
    };
}

#endif //TFDCF_TFRING_BUFFER_H
