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
#include <event/BlockingQueue.h>

int main( int argc, char *argv[] )  {
    tf::options o;
    o.register_option(tf::option("help", "Displays help", false, false, "help", 'h'));
    o.register_option(tf::option("loglevel", "Logging level (DEBUG, INFO, WARNING, ERROR)", false, true, "loglevel", 'l'));
    o.register_option(tf::option("url", "URL to connect to", true, true, "url", 'u'));
    o.register_option(tf::option("rate", "Sending message rate", false, true, "rate", 'r'));
    o.register_option(tf::option("count", "Number of messages to send", false, true, "count", 'c'));

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

    const size_t count = o.get("count", 1000ul);
    const size_t rate = o.get("rate", 100ul);

    try {
        using TimeType = std::chrono::high_resolution_clock::time_point;
        using ResultsType = std::pair<TimeType, TimeType>;
        std::vector<ResultsType> m_times(count);

        DCF::Session::initialise();

        const std::string url = o.getWithDefault("url", "");

        DCF::BlockingQueue queue;
        auto transport = fp::make_relm_connection(url.c_str(), "");

        if (!transport->valid()) {
            ERROR_LOG("Failed to create transport");
            return 1;
        }

        uint32_t id = 0;
        bool shutdown = false;

        DCF::Message sendMsg;
        sendMsg.setSubject("TEST.PERF.SOURCE");
        sendMsg.addScalarField("id", id);

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::microseconds(1000000) / rate);
        INFO_LOG("Sending a message every " << duration.count() << " us");

        queue.registerEvent(duration, [&](DCF::TimerEvent *event) noexcept {
            if (id < count) {
                sendMsg.clear();
                sendMsg.setSubject("TEST.PERF.SOURCE");
                sendMsg.addScalarField("id", id);
                m_times[id].first = std::chrono::high_resolution_clock::now();
                if (transport->sendMessage(sendMsg) == DCF::OK) {
                    DEBUG_LOG("Message send successfully: " << sendMsg);
                } else {
                    ERROR_LOG("Failed to send message");
                    exit(1);
                }
                id++;
            }
        });

        DCF::Subscriber subscriber(transport, "TEST.PERF.SINK", [&](const DCF::Subscriber *event, const DCF::Message *recvMsg) noexcept {
            DEBUG_LOG("Received message from sink: " << *recvMsg);
//            std::chrono::high_resolution_clock::time_point t = std::chrono::high_resolution_clock::now();
            uint32_t recv_id = 0;
            if (recvMsg->getScalarField("id", recv_id)) {
                DEBUG_LOG("Processing message: " << recv_id);
                uint64_t ts = 0;
                if (recvMsg->getScalarField("timestamp", ts)) {
                    std::chrono::microseconds dur(ts);
                    std::chrono::time_point<std::chrono::high_resolution_clock> t(dur);
                    m_times[recv_id].second = t;
                }
            }
            if (recv_id >= count - 1) {
                shutdown = true;
            }
        });
        queue.addSubscriber(subscriber);

        while (!shutdown) {
            queue.dispatch();
        }

        DCF::Session::destroy();

        std::vector<std::chrono::microseconds> latencies;
        latencies.reserve(count);
        std::transform(m_times.begin(), m_times.end(), std::back_inserter(latencies), [&](auto &result) {
            return std::chrono::duration_cast<std::chrono::microseconds>(result.second - result.first);
        });

        std::future<double> avg = std::async(std::launch::async, [&latencies]() {
            uint64_t total = 0;
            std::for_each(latencies.begin(), latencies.end(), [&total](auto &v) {
                total += v.count();
            });
            return static_cast<double>(total) / latencies.size();
        });

        std::future<std::chrono::microseconds> min = std::async(std::launch::async, [&latencies]() {
            return *std::min_element(latencies.begin(), latencies.end());
        });

        std::future<std::chrono::microseconds> max = std::async(std::launch::async, [&latencies]() {
            return *std::max_element(latencies.begin(), latencies.end());
        });

        auto find_p = [&](size_t p) {
            size_t index = floor((static_cast<double>(100 - p) / 100.0) * latencies.size());
            std::nth_element(latencies.begin(), latencies.begin() + index, latencies.end(), std::greater<std::chrono::microseconds>());
            return latencies[index];
        };

        std::future<std::chrono::microseconds> p99 = std::async(std::launch::async, std::bind(find_p, 95));
        std::future<std::chrono::microseconds> p90 = std::async(std::launch::async, std::bind(find_p, 90));
        std::future<std::chrono::microseconds> p50 = std::async(std::launch::async, std::bind(find_p, 50));

        INFO_LOG("Avg round trip: " << avg.get() << " us");
        INFO_LOG("Min round trip: " << min.get().count() << " us");
        INFO_LOG("Max round trip: " << max.get().count() << " us");
        INFO_LOG("P99 round trip: " << p99.get().count() << " us");
        INFO_LOG("P90 round trip: " << p90.get().count() << " us");
        INFO_LOG("P50 round trip: " << p50.get().count() << " us");

    } catch (const std::exception &stde) {
        ERROR_LOG("Internal error: " << stde.what());
        return 1;
    }
}
