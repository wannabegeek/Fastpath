//
// Created by Tom Fewster on 06/04/2016.
//

#ifndef TFDCF_FAST_LINEAR_ALLOCATOR_H
#define TFDCF_FAST_LINEAR_ALLOCATOR_H

#include <cstddef>

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
            pointer content;
            std::size_t size;
            pointer m_head;
            slab *next;
        };

        pointer m_slab;

        pointer m_head;
        pointer m_tail;
        size_t m_allocated_size = 0;

    public:
        template<typename U> struct rebind {
            typedef linear_allocator<U> other;
        };

        linear_allocator(std::size_t initial_size = 64) {
            m_slab = static_cast<char *>(malloc(initial_size * sizeof(T)));
            m_tail = m_head = m_slab;
            std::advance(m_tail, initial_size * sizeof(T));
        }

        ~linear_allocator() {
            free(m_slab);
        }

        linear_allocator(const linear_allocator &other) {
        }

        pointer allocate(std::size_t size) {
            pointer old_head = m_head;
            m_allocated_size += size;
            std::cout << "out: " << size << std::endl;
            m_head += size;
            assert(m_head <= m_tail);
            return old_head;
        }

        void deallocate(T* p, std::size_t size) {
            m_allocated_size -= size;
            std::cout << "..and back: "  << size << std::endl;
            if (m_allocated_size == 0) {
                m_head = m_slab;
                std::cout << "..and back properly" << std::endl;
            }
        }
    };

    template <class T, class U> bool operator==(const linear_allocator<T>&, const linear_allocator<U>&);
    template <class T, class U> bool operator!=(const linear_allocator<T>&, const linear_allocator<U>&);
}
#endif //TFDCF_FAST_LINEAR_ALLOCATOR_H

