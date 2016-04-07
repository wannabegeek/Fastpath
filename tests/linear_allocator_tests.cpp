//
// Created by Tom Fewster on 06/04/2016.
//

#include <gtest/gtest.h>
#include <utils/fast_linear_allocator.h>

TEST(linear_allocator, alloc) {
    using storage_alloc = std::allocator_traits<tf::linear_allocator<char>>::template rebind_alloc<char>;
    using storage_traits = std::allocator_traits<tf::linear_allocator<char>>::template rebind_traits<char>;

    storage_alloc allocator;

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

    storage_alloc allocator;

    double *ptr = reinterpret_cast<double *>(storage_traits::allocate(allocator, sizeof(double)));
    *ptr = 12.34567;

    std::cout << "ptr: " << *ptr << std::endl;
}
