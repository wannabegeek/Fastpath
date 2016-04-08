#ifndef tfthread_utils_h
#define tfthread_utils_h

#include "logger.h"
#include "../config.h"

#include <thread>
#include <vector>
#include <unistd.h>
#include <string.h>

#ifdef HAVE_PTHREAD_H
#   ifndef _GNU_SOURCE
#      define _GNU_SOURCE
#   endif
#   include <pthread.h>
#endif

namespace tf {
    class auto_thread {
    private:
        std::thread m_thread;
    public:
        auto_thread(std::thread &thread) : m_thread(std::move(thread)) {
        }

        auto_thread(std::thread &&thread) noexcept : m_thread(std::move(thread)) {
        }

        auto_thread(auto_thread &&other) noexcept : m_thread(std::move(other.m_thread)) {
        }

        auto_thread(const auto_thread &other) = delete;

        void join() {
            if (m_thread.joinable()) {
                m_thread.join();
            }
        }

        ~auto_thread() {
            if (m_thread.joinable()) {
                m_thread.join();
            }
        }

        void set_affinity(const std::vector<int> cores) {
#ifdef HAVE_PTHREAD_SETAFFINITY_NP
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

            std::for_each(cores.begin(), cores.end(), [&](const int &core) {
                if (core < num_cores) {
                    CPU_SET(core, &cpuset);
                } else {
                    ERROR_LOG("Cannot set affinity to core " << core << " only " << num_cores << " cores available");
                }
            });
            if (pthread_setaffinity_np(m_thread.native_handle(), sizeof(cpu_set_t), &cpuset) == -1) {
                ERROR_LOG("Setting CPU affinity failed: " << strerror(errno));
            }
#else
            ERROR_LOG("Setting thread affinity not supported");
#endif
        }

    };

//    namespace ThreadUtils {
////
////    public:
////        ThreadUtils() {};
////
////        ~ThreadUtils() {
////            if (m_running && m_thread.joinable()) {
////                m_thread.join();
////            }
////        }
//
//        void setName(const std::string &name, std::thread &thread) {
//#ifdef HAVE_PTHREAD_SETNAME_NP
//            pthread_setname_np(thread.native_handle(), name.c_str());
//#else
//            ERROR_LOG("Setting thread name not supported");
//#endif
//        }
//
//        void setAffinity(const size_t affinity) {
//#ifdef HAVE_PTHREAD_SETAFFINITY_NP
//            cpu_set_t cpuset;
//            CPU_ZERO(&cpuset);
//            int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
//
//            for (j = 0; j < num_cores; j++) {
//                CPU_SET(j, &cpuset);
//            }
//            pthread_setaffinity_np(std::this_thread.native_handle(), sizeof(cpu_set_t), &cpuset);
//#else
//            ERROR_LOG("Setting thread affinity not supported");
//#endif
//        }
//
//        void start() {
//
//        }
//    };
}

#endif
