//
// Created by Tom Fewster on 15/03/2016.
//

#include <iostream>
#include <utils/logger.h>
#include <utils/tfoptions.h>
#include <event/Session.h>
#include <transport/TCPTransport.h>
#include <messages/Message.h>

#define VERSION_MAJOR 0
#define VERSION_MINOR 1

int main( int argc, char *argv[] )  {
    tf::options o;
    o.register_option(tf::option("help", "Displays help", false, false, "help", 'h'));
    o.register_option(tf::option("loglevel", "Logging level (DEBUG, INFO, WARNING, ERROR)", false, true, "loglevel", 'l'));
    o.register_option(tf::option("url", "URL to connect to", true, true, "url", 'u'));
    o.register_option(tf::option("subject", "Subject to user", true, true, "subject", 's'));

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
    LOG_THREADS(true);

    try {
        DCF::Session::initialise();

        const std::string url = o.getWithDefault("url", "");
        std::string subject;
        if (!o.get("subject", subject)) {
            ERROR_LOG("You must specify a subject");
        }

        DCF::TCPTransport transport(url.c_str(), "");

        if (transport.valid()) {
            DCF::Message msg;
            msg.setSubject(subject.c_str());
            msg.addDataField("name", "tom");
            if (transport.sendMessage(msg) == DCF::OK) {
                INFO_LOG("Message send successfully");
            } else {
                ERROR_LOG("Failed to send message");
            }
        } else {
            ERROR_LOG("Failed to send message - transport not connected");
        }
        DCF::Session::destroy();
    } catch (const std::exception &stde) {
        ERROR_LOG("Internal error: " << stde.what());
        return 1;
    }
}
