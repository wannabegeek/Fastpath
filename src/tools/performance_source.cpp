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
    o.register_option(tf::option("count", "Number of messages to send", true, true, "count", 'c'));

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

    size_t count = o.get("count", 1000l);

    try {
        std::vector<std::chrono::high_resolution_clock::time_point> m_sendingTimes(count);
        std::vector<std::chrono::high_resolution_clock::time_point> m_receivingTimes(count);

        DCF::Session::initialise();

        const std::string url = o.getWithDefault("url", "");

        DCF::BlockingQueue queue;
        DCF::TCPTransport transport(url.c_str(), "");

        int id = 0;
        bool shutdown = false;

        DCF::Message sendMsg;
        sendMsg.setSubject("TEST.PERF.SOURCE");
        sendMsg.addScalarField("id", id);

        queue.registerEvent(std::chrono::milliseconds(10), [&](DCF::TimerEvent *event) {
            if (id < count) {
                sendMsg.clear();
                sendMsg.setSubject("TEST.PERF.SOURCE");
                sendMsg.addScalarField("id", ++id);
                if (transport.sendMessage(sendMsg) == DCF::OK) {
                    m_sendingTimes.push_back(std::chrono::high_resolution_clock::now());
                    DEBUG_LOG("Message send successfully: " << sendMsg);
                } else {
                    ERROR_LOG("Failed to send message");
                    exit(1);
                }
            }
        });

        queue.addSubscriber(DCF::Subscriber(&transport, "TEST.PERF.SINK", [&](const DCF::Subscriber *event, const DCF::Message *recvMsg) {
            DEBUG_LOG("Received message from sink");
            m_receivingTimes.push_back(std::chrono::high_resolution_clock::now());
            int recv_id = 0;
            if (recvMsg->getScalarField("id", recv_id)) {
                DEBUG_LOG("Processing message: " << recv_id);
            }
            if (recv_id >= count) {
                shutdown = true;
            }
        }));

        while (!shutdown) {
            queue.dispatch();
        }

        DCF::Session::destroy();

        for (id = 0; id < count; id++) {
            auto latency = std::chrono::duration_cast<std::chrono::microseconds>(m_receivingTimes[id] - m_sendingTimes[id]);
            INFO_LOG("Duration " << latency.count() << " ms");
        }

    } catch (const std::exception &stde) {
        ERROR_LOG("Internal error: " << stde.what());
        return 1;
    }
}
