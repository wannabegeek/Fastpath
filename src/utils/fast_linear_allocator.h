//
// Created by Tom Fewster on 06/04/2016.
//

#ifndef TFDCF_FAST_LINEAR_ALLOCATOR_H
#define TFDCF_FAST_LINEAR_ALLOCATOR_H

#include <cstddef>
#include <utils/logger.h>

namespace tf {
    template <typename T> class linear_allocator {
    public:
        typedef T value_type;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;

    private:

        typedef char* storage_type;

        struct slab {
            pointer m_content;
            std::size_t m_size;
            std::size_t m_allocated;
            pointer m_head;
            slab *m_next;

            static std::size_t align_up(std::size_t n) noexcept {
                static const size_t alignment = 16;
                return (n + (alignment-1)) & ~(alignment-1);
            }

            inline bool pointer_in_buffer(pointer p) const noexcept {
                return m_content <= p && p <= m_head;
            }

            slab(std::size_t size) noexcept : m_size(size), m_allocated(0), m_next(nullptr) {
                INFO_LOG("Creating new slab of size " << m_size);
                m_content = reinterpret_cast<pointer>(::malloc(align_up(m_size)));
                m_head = m_content;
            }

            ~slab() noexcept {
                ::free(m_content);
            }

            std::size_t free() const noexcept {
                return m_size - m_allocated;
            }

            pointer allocate(std::size_t size) noexcept {
                assert(this->free() >= size);
                pointer p = m_head;
                std::advance(m_head, size);
                m_allocated = std::distance(m_content, m_head);
                return p;
            }

            void deallocate(pointer ptr, std::size_t size) noexcept {
                m_allocated -= size;
                if (ptr + size == m_head) {
                    m_head = ptr;
                } else if (m_allocated == 0) {
                    m_head = m_content;
                }
                m_allocated = std::distance(m_content, m_head);
            }
        };

        slab *m_root_slab;
        slab *m_current_slab;

        std::size_t m_initial_size;

        inline slab *find_slab_with_space(slab *start, std::size_t size) const noexcept {
            if (start->free() >= size) {
                return start;
            } else if (start->m_next != nullptr) {
                return find_slab_with_space(start->m_next, size);
            }

            return nullptr;
        }

        inline slab *find_slab_containing(slab *start, T *ptr) const noexcept {
            if (ptr > start->m_content && ptr < start->m_head) {
                return start;
            } else if (start->m_next != nullptr) {
                return find_slab_containing(start->m_next, ptr);
            }

            return nullptr;
        }

    public:
        template<typename U> struct rebind {
            typedef linear_allocator<U> other;
        };

        linear_allocator(std::size_t initial_size = 1024) : m_initial_size(initial_size) {
            m_root_slab = m_current_slab = new slab(initial_size);
        }

        ~linear_allocator() {
            slab *s = m_root_slab;
            while (s != nullptr) {
                slab *m = s;
                s = s->m_next;
                delete m;
            }
        }

        linear_allocator(const linear_allocator &other) : m_initial_size(other.m_initial_size) {
            m_root_slab = m_current_slab = new slab(m_initial_size);
        }

        linear_allocator(linear_allocator &&other) : m_root_slab(other.m_root_slab), m_current_slab(other.m_current_slab), m_initial_size(other.m_initial_size) {
            other.m_root_slab = nullptr;
            other.m_current_slab = nullptr;
        }

        inline pointer allocate(const std::size_t size) noexcept {
//            INFO_LOG("Allocating " << size);
            slab *s = nullptr;
            if ((s = find_slab_with_space(m_root_slab, size)) != nullptr) {
                return s->allocate(size);
            } else {
                m_current_slab->m_next = new slab(std::max(size, m_initial_size));
                m_current_slab = m_current_slab->m_next;
                return m_current_slab->allocate(size);
            }
        }

        inline void deallocate(T* p, std::size_t size) noexcept {
            slab *s = find_slab_containing(m_root_slab, p);
            if (s != nullptr) {
                s->deallocate(p, size);
            }
//            INFO_LOG("Deallocated " << size);
        }
    };

    template <class T, class U> bool operator==(const linear_allocator<T>&, const linear_allocator<U>&);
    template <class T, class U> bool operator!=(const linear_allocator<T>&, const linear_allocator<U>&);
}
#endif //TFDCF_FAST_LINEAR_ALLOCATOR_H

