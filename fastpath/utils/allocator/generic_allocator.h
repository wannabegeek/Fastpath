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

// This is pretty much the same as the std::pmr::memory_resource ideas for C++17

#ifndef FASTPATH_GENERIC_ALLOCATOR_H
#define FASTPATH_GENERIC_ALLOCATOR_H

#include <memory>
#include <cstddef>

namespace tf {
    class allocator_resource {
    public:
        virtual ~allocator_resource() noexcept = default;

        virtual void *allocate(const std::size_t size) noexcept = 0;
        virtual void deallocate(void *p, const std::size_t size) noexcept = 0;
    };


    ////////////////
    template <typename T> class generic_allocator {
    public:
        typedef T value_type;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;
    private:
        allocator_resource *m_allocator;

    public:
        generic_allocator(allocator_resource *allocator) noexcept : m_allocator(allocator) {
        }

        generic_allocator(const generic_allocator &other) noexcept : m_allocator(other.m_allocator) {}

        generic_allocator &operator=(const generic_allocator &other) noexcept {
            m_allocator = other.m_allocator;
            return *this;
        }

        pointer allocate(const std::size_t size) noexcept {
            return static_cast<pointer>(m_allocator->allocate(sizeof(T) * size));
        }

        void deallocate(pointer p, const std::size_t size) noexcept {
            m_allocator->deallocate(reinterpret_cast<pointer>(p), sizeof(T) * size);
        }
    };

//    template <class T, class U> bool operator==(const generic_allocator<T>&, const generic_allocator<U>&);
//    template <class T, class U> bool operator!=(const generic_allocator<T>&, const generic_allocator<U>&);
}
#endif //FASTPATH_GENERIC_ALLOCATOR_H
