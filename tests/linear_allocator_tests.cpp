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

#include <gtest/gtest.h>
#include <utils/fast_linear_allocator.h>

TEST(linear_allocator, alloc) {
    using storage_alloc = std::allocator_traits<tf::linear_allocator<char>>::template rebind_alloc<char>;
    using storage_traits = std::allocator_traits<tf::linear_allocator<char>>::template rebind_traits<char>;

    storage_alloc::arena_type arena(10);
    storage_alloc allocator(arena);

    char *ptr = storage_traits::allocate(allocator, 10);
    strncpy(ptr, "0123456789", 9);

    char *ptr1 = storage_traits::allocate(allocator, 10);
    strncpy(ptr1, "abcdefghij", 9);

    std::cout << "ptr: " << ptr << std::endl;
    std::cout << "ptr1: " << ptr1 << std::endl;

    storage_traits::deallocate(allocator, ptr, 10);
    char *ptr2 = storage_traits::allocate(allocator, 10);
    strncpy(ptr2, "zyxwvutsrq", 9);
    std::cout << "ptr: " << ptr << std::endl;

    storage_traits::deallocate(allocator, ptr1, 10);
    storage_traits::deallocate(allocator, ptr2, 10);

    char *ptr3 = storage_traits::allocate(allocator, 10);
    strncpy(ptr3, "zyxwvutsrq", 9);
    std::cout << "ptr: " << ptr << std::endl;

}

TEST(linear_allocator, double_test) {
    using storage_alloc = std::allocator_traits<tf::linear_allocator<char>>::template rebind_alloc<char>;
    using storage_traits = std::allocator_traits<tf::linear_allocator<char>>::template rebind_traits<char>;

    storage_alloc::arena_type arena;
    storage_alloc allocator(arena);

    double *ptr = reinterpret_cast<double *>(storage_traits::allocate(allocator, sizeof(double)));
    *ptr = 12.34567;

    std::cout << "ptr: " << *ptr << std::endl;
}
