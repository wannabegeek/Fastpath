//
// Created by Tom Fewster on 11/03/2016.
//

#include <iostream>
#include <utils/logger.h>
#include <utils/tfoptions.h>
#include <utils/daemon_process.h>
#include "fprouter.h"
#include "bootstrap.h"


#define VERSION_MAJOR 0
#define VERSION_MINOR 1

static const char *banner = "\n+------------------------------------------------------------------------------+\n"
                                    "|   Fast Path Routing Server                                                   |\n"
                                    "|                                                                              |\n"
                                    "|   Version: %2d.%-2d                                                             |\n"
                                    "|                                                                              |\n"
                                    "|   (c) Tom Fewster <tom@wannabegeek.com> 2003-%4d                            |\n"
                                    "|   This software is covered by the GNU Public Licence, see http://www.gnu.org |\n"
                                    "|   for more details                                                           |\n"
                                    "+------------------------------------------------------------------------------+\n\n";


int main( int argc, char *argv[] )  {
    tf::options o;
    o.register_option(tf::option("help", "Displays help", false, false, "help", 'h'));
    o.register_option(tf::option("config", "Configuration file to load", true, false, "config", 'c'));
    o.register_option(tf::option("loglevel", "Logging level (DEBUG, INFO, WARNING, ERROR)", false, true, "loglevel", 'l'));
    o.register_option(tf::option("interface", "Interface to bind to", false, true, "interface", 'i'));
    o.register_option(tf::option("service", "Service to process", true, true, "service", 's'));

    o.register_option(tf::option("nobanner", "Don't display startup banner", false, false, "nobanner", 'x'));
    o.register_option(tf::option("daemon", "Run in daemon mode", false, false, "daemon", 'd'));

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

    if (!o.get("nobanner", false)) {
        char output[1024];
        sprintf(output, banner, VERSION_MAJOR, VERSION_MINOR, 2016);
        tf::logger::instance().log(tf::logger::info, output);
    }

    std::string configFile;
    o.get("config", configFile);
    DEBUG_LOG("Using config file '" << configFile << "'");

    if (o.get("daemon", false)) {
        tf::run_as_daemon();
    }

    try {
        const std::string interface = o.getWithDefault("interface", "0.0.0.0");
        const std::string service = o.getWithDefault("service", "7900");
        fp::bootstrap engine(interface, service);
        engine.run();
	} catch (const std::exception &stde) {
		ERROR_LOG("Internal error: " << stde.what());
		return 1;
	}
}
