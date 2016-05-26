/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include <iostream>

#include "fastpath/config.h"
#include "fastpath/utils/logger.h"
#include "fastpath/utils/tfoptions.h"
#include "fastpath/utils/daemon_process.h"
#include "fastpath/router/bootstrap.h"


static const char *banner = "\n+------------------------------------------------------------------------------+\n"
                                    "|   Fast Path Routing Server                                                   |\n"
                                    "|                                                                              |\n"
                                    "|   Version: %-66s|\n"
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
        sprintf(output, banner, PROJECT_VERSION, 2016);
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
