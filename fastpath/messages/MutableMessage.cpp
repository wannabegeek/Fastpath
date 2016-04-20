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

namespace fp {

    MutableMessage::MutableMessage() {
    }

    MutableMessage::MutableMessage(MutableMessage &&other) noexcept : Message(std::move(other)) {
    }

    MutableMessage::~MutableMessage() {
    }

    const bool MutableMessage::operator==(const Message &other) const {
        return Message::operator==(other);
    }

    bool MutableMessage::addScalarField(const char *field, const bool &value) {
        auto e = this->createScalarField(field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const int8_t &value) {
        auto e = this->createScalarField(field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const int16_t &value) {
        auto e = this->createScalarField(field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const int32_t &value) {
        auto e = this->createScalarField(field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const int64_t &value) {
        auto e = this->createScalarField(field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const uint8_t &value) {
        auto e = this->createScalarField(field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const uint16_t &value) {
        auto e = this->createScalarField(field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const uint32_t &value) {
        auto e = this->createScalarField(field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const uint64_t &value) {
        auto e = this->createScalarField(field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const float32_t &value) {
        auto e = this->createScalarField(field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool MutableMessage::addScalarField(const char *field, const float64_t &value) {
        auto e = this->createScalarField(field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (tf::likely(result.second)) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool MutableMessage::addDataField(const char *field, const byte *value, const size_t size) {
        DataField *e = this->createDataField(size, field, value, size);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool MutableMessage::addDataField(const char *field, const char *value) {
        DataField *e = this->createDataField(strlen(value) + 1, field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool MutableMessage::addMessageField(const char *field, BaseMessage &&msg) {
        MessageField *e = this->createMessageField(field, std::forward<BaseMessage>(msg));
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool MutableMessage::addDateTimeField(const char *field, const std::chrono::time_point<std::chrono::system_clock> &time) {
        DateTimeField *e = this->createDateTimeField(field, time);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool MutableMessage::addDateTimeField(const char *field, const std::chrono::microseconds &time) {
        DateTimeField *e = this->createDateTimeField(field, time);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool MutableMessage::addDateTimeField(const char *field, const uint64_t seconds, const uint64_t microseconds) {
        DateTimeField *e = this->createDateTimeField(field, seconds, microseconds);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
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
                this->destroyField(*it);
                m_payload.erase(it);
                return true;
            }
        }
        return false;
    }
}