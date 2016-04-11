//
// Created by fewstert on 01/04/16.
//

#include <iostream>
#include "transport/UnixSocketClient.h"
#include <thread>
#include <utils/logger.h>
#include <SharedMemoryBuffer.h>
#include "IPC/InterprocessNotifierClient.h"

int main(int argc, char *argv[]) {

    LOG_LEVEL(tf::logger::debug);

    SharedMemoryBuffer buffer("SharedMemoryTest");

    try {
        DCF::InterprocessNotifierClient notifier;
//        } else {
//            std::cerr << "Failed to create connection" << std::endl;
//        }

        unsigned int counter = 1000;
        for (unsigned int i = 0; i <= counter; i++) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            char *m = reinterpret_cast<char *>(buffer.allocate(20));
            sprintf(m, "Hello World %i", i);
            buffer.notify(m);
            INFO_LOG("Notifying " << i << "... " << std::boolalpha << notifier.notify());
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
        INFO_LOG("Exiting");
    } catch (const DCF::socket_error &e) {
        std::cerr << "BOOM - it's broken" << std::endl;
    }
}
