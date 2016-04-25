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

#ifndef FASTPATH_FAST_LINEAR_ALLOCATOR_H
#define FASTPATH_FAST_LINEAR_ALLOCATOR_H

#include <cstddef>
#include <cassert>
#include <functional>

namespace tf {

    class arena {
    public:
        using pointer = unsigned char*;
        using value_type = pointer;

    private:
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
                m_content = reinterpret_cast<pointer>(::malloc(align_up(m_size)));
                m_head = m_content;
            }

            ~slab() noexcept {
                ::free(m_content);
            }

            std::size_t free() const noexcept {
                return m_size - std::distance(m_content, m_head);
            }

            pointer allocate(std::size_t size) noexcept {
                assert(this->free() >= size);
                pointer p = m_head;
                std::advance(m_head, size);
                m_allocated += size; //std::distance(m_content, m_head);
                return p;
            }

            void deallocate(pointer ptr, std::size_t size) noexcept {
                assert(pointer_in_buffer(ptr));
                m_allocated -= size;
                assert(m_allocated >= 0);
                if (m_allocated == 0) {
                    m_head = m_content;
                } else if (ptr + size == m_head) {
                    m_head = ptr;
                }
                //m_allocated = std::distance(m_content, m_head);
            }
        };

        std::size_t m_initial_size;

        slab *m_root_slab;
        slab *m_current_slab;

        inline slab *find_slab_with_space(slab *start, std::size_t size) const noexcept {
            if (start->free() >= size) {
                return start;
            } else if (start->m_next != nullptr) {
                return find_slab_with_space(start->m_next, size);
            }

            return nullptr;
        }

        inline slab *find_slab_containing(slab *start, pointer ptr) const noexcept {
            if (start->pointer_in_buffer(ptr)) {
                return start;
            } else if (start->m_next != nullptr) {
                return find_slab_containing(start->m_next, ptr);
            }

            return nullptr;
        }

    public:
        ~arena() {
            slab *s = m_root_slab;
            while (s != nullptr) {
                s = s->m_next;
                delete s;
            }
            m_root_slab = nullptr;
        }

        arena(std::size_t initial_size = 1024) noexcept : m_initial_size(initial_size), m_root_slab(new slab(initial_size)) {
            m_current_slab = m_root_slab;
        }

        arena(const arena&) = delete;
        arena& operator=(const arena&) = delete;

        arena::pointer allocate(std::size_t size) {
            slab *s = nullptr;
            if ((s = find_slab_with_space(m_root_slab, size)) != nullptr) {
                return s->allocate(size);
            } else {
                m_current_slab->m_next = new slab(std::max(size, m_initial_size));
                m_current_slab = m_current_slab->m_next;
                return m_current_slab->allocate(size);
            }
        }

        void deallocate(arena::pointer p, std::size_t size) noexcept {
            slab *s = find_slab_containing(m_root_slab, p);
            assert(s != nullptr);
            if (s != nullptr) {
                s->deallocate(p, size);
            }
        }

        friend std::ostream &operator<<(std::ostream &out, const arena &a) {
            std::size_t block_count = 0;
            std::size_t total_free = 0;
            std::size_t total_capacity = 0;
            std::size_t total_allocated = 0;

            std::function<void(const slab *)> totals = [&](const slab *start) {
                block_count++;
                total_free += start->free();
                total_capacity += start->m_size;
                total_allocated += start->m_allocated;
                if (start->m_next != nullptr) {
                    totals(start->m_next);
                }
            };

            totals(a.m_root_slab);

            out << "allocated: " << total_allocated << " capacity: " << total_capacity << " allocatable: " << total_free << " from " << block_count << " blocks";
            return out;
        }

//        static constexpr std::size_t size() noexcept {return N;}

//        std::size_t used() const noexcept {
//            return static_cast<std::size_t>(ptr_ - buf_);
//        }

//        void reset() noexcept {ptr_ = buf_;}
    };

    template <typename T> class linear_allocator {
    public:
        typedef T value_type;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;

        using arena_type = arena;

    private:

        typedef char* storage_type;

        arena_type &m_arena;

    public:
        template<typename U> struct rebind {
            typedef linear_allocator<U> other;
        };

        linear_allocator(arena_type &arena) : m_arena(arena) {}

        ~linear_allocator() {}

        linear_allocator(const linear_allocator &other) : m_arena(other.m_arena) {}

        inline pointer allocate(const std::size_t size) noexcept {
            return reinterpret_cast<pointer>(m_arena.allocate(size));
        }

        inline void deallocate(T* p, std::size_t size) noexcept {
            m_arena.deallocate(reinterpret_cast<arena_type::pointer>(p), size);
        }
    };

    template <class T, class U> bool operator==(const linear_allocator<T>&, const linear_allocator<U>&);
    template <class T, class U> bool operator!=(const linear_allocator<T>&, const linear_allocator<U>&);
}
#endif //FASTPATH_FAST_LINEAR_ALLOCATOR_H

