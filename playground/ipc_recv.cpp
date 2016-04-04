//
// Created by fewstert on 01/04/16.
//

#include <iostream>
#include <chrono>
#include <thread>
#include <utils/logger.h>
#include <event/InlineQueue.h>
#include "IPC/InterprocessNotifierServer.h"

int main(int argc, char *argv[]) {

    LOG_LEVEL(tf::logger::debug);
//    DCF::Session::initialise();
//    DCF::InlineQueue queue;

    DCF::InlineEventManager evm;

    try {
        INFO_LOG("Started");
        DCF::InterprocessNotifierServer notifier;
        auto event = notifier.createReceiverEvent();
        evm.registerHandler(event.get());

        evm.waitForEvent();

    } catch (const DCF::socket_error &e) {
        ERROR_LOG("BOOM - it's broken: " << e.what());
    }

//    DCF::Session::m_shutdown();
}