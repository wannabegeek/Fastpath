//
// Created by Tom Fewster on 26/04/2016.
//

#ifndef FASTPATH_SHM_ALLOCATOR_H
#define FASTPATH_SHM_ALLOCATOR_H

#include <cstddef>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <boost/interprocess/managed_shared_memory.hpp>

#pragma GCC diagnostic pop

namespace tf {
    template <class T, class SegmentManager> class shm_allocator {
    private:
        SegmentManager *m_segment_manager;
    public:
    public:
        typedef T value_type;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;


        template<typename U> struct rebind {
            typedef shm_allocator<U, SegmentManager> other;
        };

        shm_allocator(SegmentManager *segment_manager) : m_segment_manager(segment_manager) {}

        ~shm_allocator() {}

        shm_allocator(const shm_allocator &other) : m_segment_manager(other.m_segment_manager) {}

        inline pointer allocate(const std::size_t size) noexcept {
            return reinterpret_cast<pointer>(m_segment_manager->allocate(size, std::nothrow));
        }

        inline void deallocate(T* p, std::size_t size) noexcept {
            m_segment_manager->deallocate(reinterpret_cast<void *>(p));
        }
    };

    template <class T, class U, class A> bool operator==(const shm_allocator<T, A>&, const shm_allocator<U, A>&);
    template <class T, class U, class A> bool operator!=(const shm_allocator<T, A>&, const shm_allocator<U, A>&);
};

#endif //FASTPATH_SHM_ALLOCATOR_H
