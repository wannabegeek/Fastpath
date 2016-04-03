//
// Created by fewstert on 01/04/16.
//

#include <iostream>
#include <chrono>
#include <thread>
#include <utils/logger.h>
#include "IPC/InterprocessNotifierServer.h"

int main(int argc, char *argv[]) {

    LOG_LEVEL(tf::logger::debug);

    try {
        INFO_LOG("Started");
        DCF::InterprocessNotifierServer notifier;
    } catch (const DCF::socket_error &e) {
        ERROR_LOG("BOOM - it's broken: " << e.what());
    }
}