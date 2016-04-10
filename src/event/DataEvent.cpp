//
// Created by fewstert on 23/03/16.
//

#include "DataEvent.h"
#include "Queue.h"

namespace DCF {

    void DataEvent::dispatch(DataEvent *event, const EventType &eventType) noexcept {
        this->__popDispatch();
        if (tf::likely(!m_pendingRemoval)) {
            m_callback(event, eventType);
        }
    }

    DataEvent::DataEvent(Queue *queue, const int fd, const EventType eventType, const std::function<void(DataEvent *, const EventType)> &callback)
            : IOEvent(queue, fd, eventType), m_callback(callback) {
    }

    DataEvent::DataEvent(DataEvent &&other) noexcept : IOEvent(std::move(other)), m_callback(std::move(other.m_callback)) {
    }

    const bool DataEvent::isEqual(const Event &other) const noexcept {
        return IOEvent::isEqual(other);
    }

    const bool DataEvent::__notify(const EventType &eventType) noexcept {
        assert(m_queue != nullptr);
        this->__pushDispatch();
        return m_queue->__enqueue(QueueElement(this, std::bind(&DataEvent::dispatch, this, this, eventType)));
    }

    void DataEvent::__destroy() noexcept {
        m_queue->unregisterEvent(this);
    }
}