//
// Created by fewstert on 23/03/16.
//

#ifndef TFDCF_TFMULTIRINGBUFFER_H
#define TFDCF_TFMULTIRINGBUFFER_H

#include <stddef.h>
#include <atomic>
#include <cassert>
#include <array>

#ifndef CACHELINE_SIZE
#   define CACHELINE_SIZE 64
#endif

namespace tf {
    template<typename T, size_t SIZE = 1024> class multiringbuffer {
        static_assert((SIZE > 1) & !(SIZE & (SIZE - 1)), "Template parameter SIZE must be a power of two.");
    private:
        struct alignas(CACHELINE_SIZE) cell_t {
            std::atomic<size_t> m_sequence;
            T m_data;
        };

        std::array<cell_t, SIZE> m_buffer;
        size_t const m_buffer_mask = SIZE - 1;
        alignas(CACHELINE_SIZE) std::atomic<size_t> m_head;
        alignas(CACHELINE_SIZE) std::atomic<size_t> m_tail;

//        multiringbuffer(multiringbuffer const&);
//        void operator=(multiringbuffer const&);
    public:
        multiringbuffer() {
            for (size_t i = 0; i != SIZE; i += 1) {
                m_buffer[i].m_sequence.store(i, std::memory_order_relaxed);
            }
            m_head.store(0, std::memory_order_relaxed);
            m_tail.store(0, std::memory_order_relaxed);
        }

        bool push(T &data) {
            cell_t* cell;
            size_t pos = m_head.load(std::memory_order_relaxed);
            for (;;) {
                cell = &m_buffer[pos & m_buffer_mask];
                size_t seq = cell->m_sequence.load(std::memory_order_acquire);
                intptr_t dif = (intptr_t)seq - (intptr_t)pos;
                if (dif == 0) {
                    if (m_head.compare_exchange_weak (pos, pos + 1, std::memory_order_relaxed)) {
                        break;
                    }
                } else if (dif < 0) {
                    return false;
                } else {
                    pos = m_head.load(std::memory_order_relaxed);
                }
            }
            cell->m_data = data;
            cell->m_sequence.store(pos + 1, std::memory_order_release);
            return true;
        }

        bool push(T &&data) {
            cell_t* cell;
            size_t pos = m_head.load(std::memory_order_relaxed);
            for (;;) {
                cell = &m_buffer[pos & m_buffer_mask];
                size_t seq = cell->m_sequence.load(std::memory_order_acquire);
                intptr_t dif = (intptr_t)seq - (intptr_t)pos;
                if (dif == 0) {
                    if (m_head.compare_exchange_weak (pos, pos + 1, std::memory_order_relaxed)) {
                        break;
                    }
                } else if (dif < 0) {
                    return false;
                } else {
                    pos = m_head.load(std::memory_order_relaxed);
                }
            }
            cell->m_data = std::move(data);
            cell->m_sequence.store(pos + 1, std::memory_order_release);
            return true;
        }

        bool pop(T &data) {
            cell_t *cell;
            size_t pos = m_tail.load(std::memory_order_relaxed);
            for (;;) {
                cell = &m_buffer[pos & m_buffer_mask];
                size_t seq = cell->m_sequence.load(std::memory_order_acquire);
                intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
                if (dif == 0) {
                    if (m_tail.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                    break;
                } else if (dif < 0) {
                    return false;
                } else {
                    pos = m_tail.load(std::memory_order_relaxed);
                }
            }
            data = cell->m_data;
            cell->m_sequence.store(pos + m_buffer_mask + 1, std::memory_order_release);
            return true;
        }
    };
}

#endif //TFDCF_TFMULTIRINGBUFFER_H
