//
// Created by Tom Fewster on 27/04/2016.
//

#include <iostream>
#include <vector>
#include "performance.h"
#include "fastpath/utils/allocator/std_allocator_resource.h"
#include "fastpath/utils/allocator/generic_allocator.h"
#include "fastpath/MutableByteStorage.h"

int main(int argc, char *argv[]) {

    using type = char;

    tf::generic_allocator<type> allocator(tf::get_default_allocator());
    fp::MutableByteStorage<type, tf::generic_allocator<type>> storage(12, allocator);

    storage.append("12345678901234567890", 21);

    std::cout << *storage << std::endl;


    std::vector<type, tf::generic_allocator<type>> v(allocator);

}