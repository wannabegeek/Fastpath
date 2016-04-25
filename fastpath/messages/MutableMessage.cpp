//
// Created by Tom Fewster on 20/04/2016.
//

#include "fastpath/messages/MutableMessage.h"

#include "fastpath/messages/ScalarField.h"
#include "fastpath/messages/DataField.h"
#include "fastpath/messages/DateTimeField.h"
#include "fastpath/messages/SmallDataField.h"
#include "fastpath/messages/LargeDataField.h"
#include "fastpath/messages/MessageField.h"
#include "FieldAllocator.h"

namespace fp {

    MutableMessage::MutableMessage() noexcept {
    }

    MutableMessage::MutableMessage(MutableMessage &&other) noexcept : Message(std::move(other)) {
    }

    MutableMessage::~MutableMessage() noexcept {
    }

    const bool MutableMessage::operator==(const Message &other) const {
        return Message::operator==(other);
    }

    const bool MutableMessage::setSubject(const char *subject) noexcept {
        if (strlen(subject) < max_subject_length) {
            strcpy(&m_subject[0], subject);
            return true;
        }

        return false;
    }

    bool MutableMessage::addScalarField(const char *field, const bool &value) {
        auto e = createScalarField(m_field_allocator, field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            destroyField(m_field_allocator, e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const int8_t &value) {
        auto e = createScalarField(m_field_allocator, field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            destroyField(m_field_allocator, e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const int16_t &value) {
        auto e = createScalarField(m_field_allocator, field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            destroyField(m_field_allocator, e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const int32_t &value) {
        auto e = createScalarField(m_field_allocator, field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            destroyField(m_field_allocator, e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const int64_t &value) {
        auto e = createScalarField(m_field_allocator, field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            destroyField(m_field_allocator, e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const uint8_t &value) {
        auto e = createScalarField(m_field_allocator, field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            destroyField(m_field_allocator, e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const uint16_t &value) {
        auto e = createScalarField(m_field_allocator, field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            destroyField(m_field_allocator, e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const uint32_t &value) {
        auto e = createScalarField(m_field_allocator, field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            destroyField(m_field_allocator, e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const uint64_t &value) {
        auto e = createScalarField(m_field_allocator, field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            destroyField(m_field_allocator, e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const float32_t &value) {
        auto e = createScalarField(m_field_allocator, field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            destroyField(m_field_allocator, e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const float64_t &value) {
        auto e = createScalarField(m_field_allocator, field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            destroyField(m_field_allocator, e);
        }
        return result.second;
    }

    bool MutableMessage::addDataField(const char *field, const byte *value, const size_t size) {
        DataField *e = createDataField(m_field_allocator, size, field, value, size);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            destroyField(m_field_allocator, e);
        }
        return result.second;
    }

    bool MutableMessage::addDataField(const char *field, const char *value) {
        DataField *e = createDataField(m_field_allocator, strlen(value) + 1, field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            destroyField(m_field_allocator, e);
        }
        return result.second;
    }

    bool MutableMessage::addMessageField(const char *field, BaseMessage &&msg) {
        MessageField *e = createMessageField(m_field_allocator, field, std::forward<BaseMessage>(msg));
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            destroyField(m_field_allocator, e);
        }
        return result.second;
    }

    bool MutableMessage::addDateTimeField(const char *field, const std::chrono::time_point<std::chrono::system_clock> &time) {
        DateTimeField *e = createDateTimeField(m_field_allocator, field, time);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            destroyField(m_field_allocator, e);
        }
        return result.second;
    }

    bool MutableMessage::addDateTimeField(const char *field, const std::chrono::microseconds &time) {
        DateTimeField *e = createDateTimeField(m_field_allocator, field, time);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            destroyField(m_field_allocator, e);
        }
        return result.second;
    }

    bool MutableMessage::addDateTimeField(const char *field, const uint64_t seconds, const uint64_t microseconds) {
        DateTimeField *e = createDateTimeField(m_field_allocator, field, seconds, microseconds);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            destroyField(m_field_allocator, e);
        }
        return result.second;
    }

    bool MutableMessage::removeField(const char* field) {
        if (field != nullptr) {
            auto index = m_keys.find(field);
            if (index != m_keys.end()) {
                m_keys.erase(field);
                auto it = m_payload.begin();
                std::advance(it, index->second);
                destroyField(m_field_allocator, *it);
                m_payload.erase(it);
                return true;
            }
        }
        return false;
    }
}
