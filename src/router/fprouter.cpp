//
// Created by Tom Fewster on 11/03/2016.
//

#include <iostream>
#include <utils/logger.h>
#include <utils/tfoptions.h>
#include "fprouter.h"

#define VERSION 0.1

int main( int argc, char *argv[] )  {
    tf::options o;
    o.register_option(tf::option("help", "Displays help", false, false, "help", 'h'));
    o.register_option(tf::option("config", "Configuration file to load", true, false, "config", 'c'));
    o.register_option(tf::option("loglevel", "Logging level (DEBUG, INFO, WARNING, ERROR)", false, true, "loglevel", 'l'));
    o.register_option(tf::option("nobanner", "Don't display startup banner", false, false, "nobanner", 'x'));

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
        const std::ostringstream banner << std::endl << "+------------------------------------------------------------------------------+" << std::endl
                   << "|   Distributed Communication Server                                           |" << std::endl
                   << "|                                                                              |" << std::endl
                   << "|   Version: " << VERSION << "                                                              |" << std::endl
                   << "|                                                                              |" << std::endl
                   << "|   (c) Tom Fewster <tom@wannabegeek.com> 2003-2006                            |" << std::endl
                   << "|   This software is covered by the GNU Public Licence, see http://www.gnu.org |" << std::endl
                   << "|      for more details                                                        |" << std::endl
                   << "+------------------------------------------------------------------------------+" << std::endl);
        tf::logger::instance().log(tf::logger::info, banner.str());

    }

    std::string configFile;
    o.get("config", configFile);
    DEBUG_LOG("Using config file '" << configFile << "'");

//	try {
//		TF::ExecutionEngineBootstrap executionEngine(configFile, !nosignals);
//
//		// now that we should be fully initialised & all objects created, we can call run.
//		// This will first link objects together, then run the man dispatch loop
//		executionEngine.run();
//	} catch (const TF::Exception &e) {
//		WARNING_LOG("Exiting");
//		return 1;
//	} catch (const std::exception &stde) {
//		ERROR_LOG("Unhandled Exception Caught: " << stde.what());
//		return 2;
//	}
}
