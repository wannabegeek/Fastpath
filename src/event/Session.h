//
// Created by Tom Fewster on 02/03/2016.
//

#ifndef TFDCF_SESSION_H
#define TFDCF_SESSION_H

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <status.h>
#include "GlobalEventManager.h"

namespace DCF {
    class Session {
    private:
        bool m_started;
        std::atomic_bool m_shutdown = ATOMIC_VAR_INIT(false);
        GlobalEventManager m_eventManager;

        std::thread m_eventLoop;

        Session() : m_started(false) {}

        const status start() {
            if (m_started) {
                return OK;
            }

            std::mutex mutex;
            std::condition_variable condition;

            std::unique_lock<std::mutex> lock(mutex);
            m_eventLoop = std::thread([&]() {
                {
                    std::lock_guard<std::mutex> lock_guard(mutex);
                    condition.notify_all();
                }
                while (!m_shutdown.load()) {
                    m_eventManager.waitForEvent();
                }

                m_shutdown.store(false);
                DEBUG_LOG("Event loop exit");
            });

            condition.wait(lock);

            return OK;
        }

        const status stop() {
            if (m_started) {
                return EVM_NOTRUNNING;
            }

            m_shutdown.store(true);
            m_eventManager.notify();
            if (m_eventLoop.joinable()) {
                m_eventLoop.join();
            }

            m_started = false;
            return OK;
        }

        static Session &instance() {
            static Session s_instance;
            return s_instance;
        }

    public:
        static status initialise() {
            return Session::instance().start();
        }

        static status destroy() {
            return Session::instance().stop();
        }

        friend class Queue;
    };
}

#endif //TFDCF_SESSION_H
