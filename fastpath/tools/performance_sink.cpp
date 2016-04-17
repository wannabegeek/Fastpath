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

#include "fastpath/utils/tfoptions.h"
#include "fastpath/fastpath.h"

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

        auto transport = fp::make_relm_connection(url.c_str(), "");

        if (!transport->valid()) {
            ERROR_LOG("Failed to create transport");
            return 1;
        }

        DCF::Message sendMsg;
        DCF::Subscriber subscriber(transport, "TEST.PERF.SOURCE", [&](const DCF::Subscriber *event, const DCF::Message *msg) noexcept {
            auto recv_ts = std::chrono::high_resolution_clock::now();
            DEBUG_LOG("Received message: " << *msg);
            uint32_t id = 0;
            if (msg->getScalarField("id", id)) {
                DEBUG_LOG("Processing message: " << id);
                sendMsg.clear();
                sendMsg.setSubject("TEST.PERF.SINK");
                sendMsg.addScalarField("id", id);
                const uint64_t ts = std::chrono::duration_cast<std::chrono::microseconds>(recv_ts.time_since_epoch()).count();
                sendMsg.addScalarField("timestamp", ts);
                if (transport->sendMessage(sendMsg) == DCF::OK) {
                    DEBUG_LOG("Message send successfully: " << sendMsg);
                } else {
                    ERROR_LOG("Failed to send message");
                    exit(1);
                }
            } else {
                ERROR_LOG("Message did not contain 'id' tag");
            }
        });
        queue.addSubscriber(subscriber);

        while (true) {
            queue.dispatch();
        }
    } catch (const std::exception &stde) {
        ERROR_LOG("Internal error: " << stde.what());
        return 1;
    }
}