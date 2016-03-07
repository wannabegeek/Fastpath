#ifndef tflogger_h
#define tflogger_h

#include "config.h"
#include "console_colors.h"

#include "tfspinlock.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <stdio.h>
#include <thread>
#include <cstring>
#include <functional>
#include <type_traits>

namespace tf {
    static const char description[] {
            'E', // tfLogLevelError
            'W', // tfLogLevelWarning
            'I', // tfLogLevelInfo
            'D', // tfLogLevelDebug
            'T'  // tfLogLevelTrace
    };

    static const tf::setcolor colors[] {
            tf::setcolor(tf::color::red, tf::attr::bright),
            tf::setcolor(tf::color::yellow, tf::attr::normal),
            tf::setcolor(tf::color::white, tf::attr::normal),
            tf::setcolor(tf::color::white, tf::attr::dim),
            tf::setcolor(tf::color::red, tf::attr::dim)
    };

    class logger {
    public:
        typedef enum {
            error = 0,
            warning,
            info,
            debug,
            trace
        } level;

        level logLevel;

    private:
        logger() : m_supports_colors(tf::setcolor::supported_output_device()), m_log_thread_id(false) {
            use_colors(m_supports_colors);
        };
        logger(const logger &) = delete;
        logger &operator=(const logger &rhs) = delete;

        tf::spinlock m_lock;
        const bool m_supports_colors;
        bool m_log_thread_id;

        std::function<void (const logger::level &, const char *, const std::string &)> m_outputfn;

    protected:
        std::ostream &m_output_stream = std::cout;
    public:
        static inline logger &instance() noexcept {
            static logger instance;
            return instance;
        };

        void use_colors(const bool value) {
            if (value && m_supports_colors) {
                m_outputfn = [&](const logger::level &level, const char *prefix, const std::string &msg) {
                    if (m_log_thread_id) {
                        m_output_stream << description[level] << tf::setcolor(tf::color::yellow) << prefix << tf::setcolor(tf::color::green) << std::this_thread::get_id() << ": " << colors[level] << msg << tf::setcolor(tf::color::normal) << std::endl;
                    } else {
                        m_output_stream << description[level] << tf::setcolor(tf::color::yellow) << prefix << colors[level] << msg << tf::setcolor(tf::color::normal) << std::endl;
                    }
                };;
            } else {
                m_outputfn = [&](const logger::level &level, const char *prefix, const std::string &msg) {
                    if (m_log_thread_id) {
                        m_output_stream << description[level] << prefix << std::this_thread::get_id() << ": " << msg << std::endl;
                    } else {
                        m_output_stream << description[level] << prefix << msg << std::endl;
                    }
                };;
            }
        }

        void log_thread_id(const bool value) {
            m_log_thread_id = value;
        }

        inline void log(const logger::level &level, const std::string &msg) noexcept {
            std::chrono::system_clock::time_point time = std::chrono::system_clock::now();
            auto usec = std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()).count() % 1000000;
            std::time_t now_c = std::chrono::system_clock::to_time_t(time);
            std::tm tm = *std::localtime(&now_c);

            static const char *timeFormat = "%H:%M:%S";

            char mbstr[9] = {0};
            std::strftime(mbstr, 9, timeFormat, &tm);
            char prefix[25] = {0};
            sprintf(prefix, ": [%s.%06lli] ", mbstr, static_cast<long long>(usec));

            m_lock.lock();
            m_outputfn(level, prefix, msg);
            m_lock.unlock();
        };

        inline void log(const logger::level &level, const std::ostringstream &msg) noexcept {
            log(level, msg.str());
        }
    };

}

#define LOG(level, msg) { \
	if (level <= tf::logger::instance().logLevel) { \
		std::ostringstream mess; \
		mess << msg; \
		tf::logger::instance().log(level, mess.str()); \
	} \
}

#ifdef TRACE
#   define TRACE_LOG(msg) { LOG(tf::logger::Trace, msg); }
#else
#   define TRACE_LOG(msg)
#endif
#ifdef DEBUG
#   define DEBUG_LOG(msg) { LOG(tf::logger::debug, msg); }
#else
#   define DEBUG_LOG(msg)
#endif
#define INFO_LOG(msg) { LOG(tf::logger::info, msg); }
#define WARNING_LOG(msg) { LOG(tf::logger::warning, msg); }
#define ERROR_LOG(msg) { LOG(tf::logger::error, msg); }

#define LOG_LEVEL(level) { tf::logger::instance().logLevel = level; }
#define LOG_COLOR(yn) { tf::logger::instance().use_colors(yn); }
#define LOG_THREADS(yn) { tf::logger::instance().log_thread_id(yn); }

#endif
