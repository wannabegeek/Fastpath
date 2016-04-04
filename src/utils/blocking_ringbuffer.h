//
// Created by Tom Fewster on 10/03/2016.
//

#ifndef TFDCF_BLOCKING_RINGBUFFER_H
#define TFDCF_BLOCKING_RINGBUFFER_H

#include "tfringbuffer.h"
#include "semaphore_light.h"
#include <type_traits>

namespace tf {
    template <typename T, size_t SIZE = 1024> class blocking_ringbuffer final : public ringbuffer<T, SIZE> {
        LightweightSemaphore m_semaphore;

    public:
        void pop_wait(T &object) {
            while (!this->pop(object)) {
                m_semaphore.wait();
            }
        }

        virtual bool push(const T &&object) override {
            bool v = ringbuffer<T, SIZE>::push(std::move(object));
            m_semaphore.signal();
            return v;
        }

        virtual bool push(const T &object) override {
            bool v = ringbuffer<T, SIZE>::push(object);
            m_semaphore.signal();
            return v;
        }
    };
}

#endif //TFDCF_BLOCKING_RINGBUFFER_H
