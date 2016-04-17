#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <cstring>
#include <cstdlib>
#include <string>
#include <thread>
#include <iostream>
#include "fastpath/SharedMemoryBuffer.h"

int main(int argc, char *argv[])
{
    boost::interprocess::shared_memory_object::remove("SharedMemoryTest");
    SharedMemoryBuffer buffer("SharedMemoryTest");

    //Write all the memory to 1
    unsigned int counter = 1000;
    for (unsigned int i = 0; i <= counter; i++) {
        char *m = reinterpret_cast<char *>(buffer.allocate(20));
        sprintf(m, "Hello World %i", i);
        buffer.notify(m);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    };
}