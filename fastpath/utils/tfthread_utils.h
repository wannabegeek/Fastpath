/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 15/06/2016

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

#ifndef tfthread_utils_h
#define tfthread_utils_h

#include "fastpath/utils/logger.h"
#include "fastpath/config.h"

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
    namespace thread {
        bool can_set_affinity() noexcept {
#ifdef HAVE_PTHREAD_SETAFFINITY_NP
            return true;
#else
            return false;
#endif
        }

        int get_num_cores() noexcept {
            // This give the total number of addressable cores, which may include hyper-threading
            static int num_cores = std::thread::hardware_concurrency();
            return num_cores;
        }

        void set_affinity(std::thread &thread, const std::initializer_list<int> cores) {
#ifdef HAVE_PTHREAD_SETAFFINITY_NP
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            const int num_cores = get_num_cores();

            for (auto &core : cores) {
                if (core < num_cores) {
                    CPU_SET(core, &cpuset);
                } else {
                    ERROR_LOG("Cannot set affinity to core " << core << " only " << num_cores << " cores available");
                }
            });
            if (pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpuset) == -1) {
                ERROR_LOG("Setting CPU affinity failed: " << strerror(errno));
            }
#else
            ERROR_LOG("Setting thread affinity not supported");
#endif
        }
    }

//    class auto_thread {
//    private:
//        std::thread m_thread;
//    public:
//        auto_thread(std::thread &thread) : m_thread(std::move(thread)) {
//        }
//
//        auto_thread(std::thread &&thread) noexcept : m_thread(std::move(thread)) {
//        }
//
//        auto_thread(auto_thread &&other) noexcept : m_thread(std::move(other.m_thread)) {
//        }
//
//        auto_thread(const auto_thread &other) = delete;
//
//        void join() {
//            if (m_thread.joinable()) {
//                m_thread.join();
//            }
//        }
//
//        ~auto_thread() {
//            if (m_thread.joinable()) {
//                m_thread.join();
//            }
//        }
//
//        void set_affinity(const std::vector<int> cores) {
//#ifdef HAVE_PTHREAD_SETAFFINITY_NP
//            cpu_set_t cpuset;
//            CPU_ZERO(&cpuset);
//            int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
//
//            std::for_each(cores.begin(), cores.end(), [&](const int &core) {
//                if (core < num_cores) {
//                    CPU_SET(core, &cpuset);
//                } else {
//                    ERROR_LOG("Cannot set affinity to core " << core << " only " << num_cores << " cores available");
//                }
//            });
//            if (pthread_setaffinity_np(m_thread.native_handle(), sizeof(cpu_set_t), &cpuset) == -1) {
//                ERROR_LOG("Setting CPU affinity failed: " << strerror(errno));
//            }
//#else
//            ERROR_LOG("Setting thread affinity not supported");
//#endif
//        }
//
//    };

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
