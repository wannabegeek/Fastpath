//
// Created by fewstert on 01/04/16.
//

#include <iostream>
#include "transport/UnixSocketClient.h"
#include <thread>
#include <utils/logger.h>
#include "IPC/InterprocessNotifierClient.h"

int main(int argc, char *argv[]) {

    LOG_LEVEL(tf::logger::debug);

    try {
        DCF::InterprocessNotifierClient notifier;
//        } else {
//            std::cerr << "Failed to create connection" << std::endl;
//        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
        INFO_LOG("Notifying 1... " << std::boolalpha << notifier.notify());

        std::this_thread::sleep_for(std::chrono::seconds(2));
        INFO_LOG("Notifying 2... " << std::boolalpha << notifier.notify());

        std::this_thread::sleep_for(std::chrono::seconds(2));
        INFO_LOG("Notifying 3... " << std::boolalpha << notifier.notify());

        std::this_thread::sleep_for(std::chrono::seconds(2));
        INFO_LOG("Exiting");
    } catch (const DCF::socket_error &e) {
        std::cerr << "BOOM - it's broken" << std::endl;
    }
}
