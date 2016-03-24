//
// Created by Tom Fewster on 23/03/2016.
//

#include <iostream>
#include <utils/logger.h>
#include <utils/tfoptions.h>
#include <event/Session.h>
#include <transport/TCPTransport.h>
#include <messages/Message.h>
#include <event/BlockingQueue.h>

int main( int argc, char *argv[] )  {
    tf::options o;
    o.register_option(tf::option("help", "Displays help", false, false, "help", 'h'));
    o.register_option(tf::option("loglevel", "Logging level (DEBUG, INFO, WARNING, ERROR)", false, true, "loglevel", 'l'));
    o.register_option(tf::option("url", "URL to connect to", true, true, "url", 'u'));

    try {
        o.parse(argc, argv);
    } catch (const tf::option_exception &e) {
        ERROR_LOG(e.what());
        o.printUsage();
        return 1;
    }

    std::string loglevel;
    if (o.get("loglevel", loglevel)) {
        if (loglevel == "DEBUG") {
            LOG_LEVEL(tf::logger::debug);
        } else if (loglevel == "INFO") {
            LOG_LEVEL(tf::logger::info);
        } else if (loglevel == "WARNING") {
            LOG_LEVEL(tf::logger::warning);
        } else if (loglevel == "ERROR") {
            LOG_LEVEL(tf::logger::error);
        } else {
            ERROR_LOG("Invalid log level");
            return 1;
        }
    } else {
        LOG_LEVEL(tf::logger::warning);
    }

    try {
        DCF::Session::initialise();

        const std::string url = o.getWithDefault("url", "");

        DCF::BlockingQueue queue;
        DCF::TCPTransport transport(url.c_str(), "");

        DCF::Message sendMsg;
        queue.addSubscriber(DCF::Subscriber(&transport, "TEST.PERF.SOURCE", [&](const DCF::Subscriber *event, const DCF::Message *msg) {
            DEBUG_LOG("Received message: " << *msg);
            int id = 0;
            if (msg->getScalarField("id", id)) {
                DEBUG_LOG("Processing message: " << id);
            }
            sendMsg.clear();
            sendMsg.setSubject("TEST.PERF.SINK");
            sendMsg.addScalarField("id", id);
            if (transport.sendMessage(sendMsg) == DCF::OK) {
                DEBUG_LOG("Message send successfully: " << sendMsg);
            } else {
                ERROR_LOG("Failed to send message");
                return 1;
            }
        }));

        while (true) {
            queue.dispatch();
        }
    } catch (const std::exception &stde) {
        ERROR_LOG("Internal error: " << stde.what());
        return 1;
    }
}