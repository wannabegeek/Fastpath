//
// Created by Tom Fewster on 29/02/2016.
//

#ifndef TFDCF_QUEUE_H
#define TFDCF_QUEUE_H

#include "messages/StorageTypes.h"

namespace DCF {
    class Event;

    class Queue {
    public:
        using queue_value_type = std::function<void ()>;

        virtual ~Queue() {}

        virtual void dispatch() = 0;
        virtual void dispatch(const std::chrono::microseconds &timeout) = 0;
        virtual const bool try_dispatch() = 0;
        virtual const size_t eventsInQueue() const noexcept = 0;
        virtual const bool enqueue(queue_value_type &event) noexcept = 0;
    };
}

#endif //TFDCF_QUEUE_H
