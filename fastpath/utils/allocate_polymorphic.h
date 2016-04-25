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

#ifndef FASTPATH_ALLOCATE_POLYMORPHIC_H
#define FASTPATH_ALLOCATE_POLYMORPHIC_H

#include <type_traits>

/**
 * Simple allocator for handling polymorphic types
 * It stores extra bytes at the start of the allocation to keep track of the allocation size.
 */

namespace tf {
    struct allocate_polymorphic {
//        template <typename T, typename Allocator, class ...Args, typename std::enable_if<std::is_nothrow_constructible<T>::value>::true> inline static T *allocate(Allocator &allocator, Args &&...args) {
//            typename std::allocator_traits<Allocator>::pointer ptr = std::allocator_traits<Allocator>::allocate(allocator, sizeof(T) + sizeof(std::size_t));
//            std::size_t *info = reinterpret_cast<std::size_t *>(ptr);
//
//            *info = sizeof(T);
//            std::advance(ptr, sizeof(std::size_t));
//            return new(ptr) T(std::forward<Args>(args)...);
//        }

        template <typename T, typename Allocator, class ...Args> inline static T *allocate(Allocator &allocator, Args &&...args) {
            typename std::allocator_traits<Allocator>::pointer ptr = std::allocator_traits<Allocator>::allocate(allocator, sizeof(T) + sizeof(std::size_t));
            std::size_t *info = reinterpret_cast<std::size_t *>(ptr);

            *info = sizeof(T);
            std::advance(ptr, sizeof(std::size_t));
            return new(ptr) T(std::forward<Args>(args)...);
        }

        template <typename T, typename Allocator> inline static void deallocate(Allocator &allocator, T *ptr) noexcept {
            ptr->~T();

            typename std::allocator_traits<Allocator>::pointer alloc_ptr = reinterpret_cast<typename std::allocator_traits<Allocator>::pointer>(ptr);
            std::advance(alloc_ptr, -sizeof(std::size_t));
            std::size_t *info = reinterpret_cast<std::size_t *>(alloc_ptr);
            *info += sizeof(std::size_t);
            std::allocator_traits<Allocator>::deallocate(allocator, alloc_ptr, *info);
        }
    };
}

#endif //FASTPATH_ALLOCATE_POLYMORPHIC_H
