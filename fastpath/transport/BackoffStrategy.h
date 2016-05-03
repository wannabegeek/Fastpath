//
// Created by Tom Fewster on 02/05/2016.
//

#ifndef FASTPATH_BACKOFFSTRATEGY_H
#define FASTPATH_BACKOFFSTRATEGY_H

#include <chrono>
#include <thread>

namespace fp {
    class BackoffStrategy {
    private:
        static constexpr std::chrono::milliseconds min_retry_interval() { return std::chrono::milliseconds(1); }
        static constexpr std::chrono::milliseconds max_retry_interval() { return std::chrono::seconds(5); }

        std::chrono::milliseconds m_current;
    public:
        BackoffStrategy() : m_current(min_retry_interval()) {}

        void backoff() noexcept {
            std::this_thread::sleep_for(m_current);
            m_current = std::min(m_current * 2, max_retry_interval());
        }
    };
}
#endif //FASTPATH_BACKOFFSTRATEGY_H
