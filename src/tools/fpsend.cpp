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
#include <utils/logger.h>
#include <utils/tfoptions.h>
#include <event/Session.h>
#include <transport/realm_transport.h>
#include <messages/Message.h>

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

    try {
        DCF::Session::initialise();

        const std::string url = o.getWithDefault("url", "");
        std::string subject;
        if (!o.get("subject", subject)) {
            ERROR_LOG("You must specify a subject");
        }

        auto transport = fp::make_relm_connection(url.c_str(), "");

        if (transport->valid()) {
            DCF::Message msg;
            msg.setSubject(subject.c_str());
            msg.addDataField("name", "tom");
            if (transport->sendMessage(msg) == DCF::OK) {
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
