//
// Created by fewstert on 01/04/16.
//

#include <iostream>
#include "fastpath/transport/UnixSocketClient.h"
#include <thread>
#include "fastpath/utils/logger.h"
#include "fastpath/SharedMemoryBuffer.h"
#include "IPC/InterprocessNotifierClient.h"

int main(int argc, char *argv[]) {

    LOG_LEVEL(tf::logger::debug);

    SharedMemoryBuffer buffer("SharedMemoryTest2");

    try {
        fp::InterprocessNotifierClient notifier;
//        } else {
//            std::cerr << "Failed to create connection" << std::endl;
//        }

        unsigned int counter = 1000;
        SharedMemoryBuffer::mutable_storage_type storage(2048, buffer.allocator());
        for (unsigned int i = 0; i <= counter; i++) {
            std::this_thread::sleep_for(std::chrono::seconds(2));

            char m[255];
            sprintf(m, "Hello World %i", i);
            storage.append(reinterpret_cast<byte *>(&m[0]), strlen(m));
            buffer.notify(&storage);
            INFO_LOG("Notifying " << i << "... " << std::boolalpha << notifier.notify());
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
        INFO_LOG("Exiting");
    } catch (const fp::socket_error &e) {
        std::cerr << "BOOM - it's broken" << std::endl;
    }
}
