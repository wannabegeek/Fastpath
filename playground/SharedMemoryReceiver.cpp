#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <cstring>
#include <cstdlib>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include "fastpath/SharedMemoryBuffer.h"

int main(int argc, char *argv[])
{
    SharedMemoryBuffer buffer("SharedMemoryTest");

    while (true) {
        if (buffer.size()) {
            void *ptr = buffer.retrieve();
            std::cout << reinterpret_cast<const char *>(ptr) << std::endl;
            buffer.deallocate(ptr);
        } else {
            std::cout << "." << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}
