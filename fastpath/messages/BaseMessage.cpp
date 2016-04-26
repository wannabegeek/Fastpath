/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

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

#include "fastpath/messages/BaseMessage.h"

#include "fastpath/utils/logger.h"
#include "fastpath/messages/DataField.h"
#include "fastpath/messages/DateTimeField.h"
#include "fastpath/messages/MessageField.h"

#include <iostream>
#include "FieldAllocator.h"

namespace fp {

    BaseMessage::BaseMessage(BaseMessage &&other) noexcept : //m_arena(std::move(other.m_arena)),
                                                             m_field_allocator(other.m_field_allocator),
                                                             m_payload(std::move(other.m_payload)),
                                                             m_keys(std::move(other.m_keys)) {
        other.m_payload.clear();
        other.m_keys.clear();
    }

    BaseMessage::~BaseMessage() noexcept {
        for (Field *field : m_payload) {
            destroyField(m_field_allocator, field);
        }
    }

    void BaseMessage::clear() noexcept {
        std::for_each(m_payload.begin(), m_payload.end(), [this](Field *field) {
                destroyField(m_field_allocator, field);
        });
        m_payload.clear();
        m_keys.clear();
    }

    void BaseMessage::detach() noexcept {
    }

    const bool BaseMessage::operator==(const BaseMessage &other) const {
        return std::equal(m_payload.begin(), m_payload.end(), other.m_payload.begin(), [](const PayloadContainer::value_type& item1, const PayloadContainer::value_type& item2) -> bool {
                        return *item1 == *item2;
                });
    }

    const storage_type BaseMessage::storageType(const char *field) const noexcept {
        const Field *element = m_payload[m_keys.at(field)];
        return element->type();
    }

    bool BaseMessage::getScalarField(const char *field, bool &value) const noexcept {
        if (field != nullptr) {
            auto index = m_keys.find(field);
            if (index != m_keys.end()) {
                const ScalarField *element = reinterpret_cast<ScalarField *>(
                        m_payload[index->second]);
                value = element->get<bool>();
                return true;
            }
        }
        return false;
    }

    bool BaseMessage::getScalarField(const char *field, int8_t &value) const noexcept {
        if (field != nullptr) {
            auto index = m_keys.find(field);
            if (index != m_keys.end()) {
                const ScalarField *element = reinterpret_cast<ScalarField *>(
                        m_payload[index->second]);
                value = element->get<int8_t>();
                return true;
            }
        }
        return false;
    }

    bool BaseMessage::getScalarField(const char *field, int16_t &value) const noexcept {
        if (field != nullptr) {
            auto index = m_keys.find(field);
            if (index != m_keys.end()) {
                const ScalarField *element = reinterpret_cast<ScalarField *>(
                        m_payload[index->second]);
                value = element->get<int16_t>();
                return true;
            }
        }
        return false;
    }

    bool BaseMessage::getScalarField(const char *field, int32_t &value) const noexcept {
        if (field != nullptr) {
            auto index = m_keys.find(field);
            if (index != m_keys.end()) {
                const ScalarField *element = reinterpret_cast<ScalarField *>(
                        m_payload[index->second]);
                value = element->get<int32_t>();
                return true;
            }
        }
        return false;
    }

    bool BaseMessage::getScalarField(const char *field, int64_t &value) const noexcept {
        if (field != nullptr) {
            auto index = m_keys.find(field);
            if (index != m_keys.end()) {
                const ScalarField *element = reinterpret_cast<ScalarField *>(
                        m_payload[index->second]);
                value = element->get<int64_t>();
                return true;
            }
        }
        return false;
    }

    bool BaseMessage::getScalarField(const char *field, uint8_t &value) const noexcept {
        if (field != nullptr) {
            auto index = m_keys.find(field);
            if (index != m_keys.end()) {
                const ScalarField *element = reinterpret_cast<ScalarField *>(
                        m_payload[index->second]);
                value = element->get<uint8_t>();
                return true;
            }
        }
        return false;
    }

    bool BaseMessage::getScalarField(const char *field, uint16_t &value) const noexcept {
        if (field != nullptr) {
            auto index = m_keys.find(field);
            if (index != m_keys.end()) {
                const ScalarField *element = reinterpret_cast<ScalarField *>(
                        m_payload[index->second]);
                value = element->get<uint16_t>();
                return true;
            }
        }
        return false;
    }

    bool BaseMessage::getScalarField(const char *field, uint32_t &value) const noexcept {
        if (field != nullptr) {
            auto index = m_keys.find(field);
            if (index != m_keys.end()) {
                const ScalarField *element = reinterpret_cast<ScalarField *>(
                        m_payload[index->second]);
                value = element->get<uint32_t>();
                return true;
            }
        }
        return false;
    }

    bool BaseMessage::getScalarField(const char *field, uint64_t &value) const noexcept {
        if (field != nullptr) {
            auto index = m_keys.find(field);
            if (index != m_keys.end()) {
                const ScalarField *element = reinterpret_cast<ScalarField *>(
                        m_payload[index->second]);
                value = element->get<uint64_t>();
                return true;
            }
        }
        return false;
    }

    bool BaseMessage::getScalarField(const char *field, float32_t &value) const noexcept {
        if (field != nullptr) {
            auto index = m_keys.find(field);
            if (index != m_keys.end()) {
                const ScalarField *element = reinterpret_cast<ScalarField *>(
                        m_payload[index->second]);
                value = element->get<float32_t>();
                return true;
            }
        }
        return false;
    }

    bool BaseMessage::getScalarField(const char *field, float64_t &value) const noexcept {
        if (field != nullptr) {
            auto index = m_keys.find(field);
            if (index != m_keys.end()) {
                const ScalarField *element = reinterpret_cast<ScalarField *>(
                        m_payload[index->second]);
                value = element->get<float64_t>();
                return true;
            }
        }
        return false;
    }


    bool BaseMessage::getDataField(const char *field, const char **value, size_t &length) const {
        if (field != nullptr) {
            auto index = m_keys.find(field);
            if (index != m_keys.end()) {
                const DataField *element = reinterpret_cast<DataField *>(m_payload[index->second]);
                length = element->get(value);
                return true;
            }
        }
        return false;
    }

    void BaseMessage::prepareForReuse() {
        this->clear();
    }

    std::ostream &BaseMessage::output(std::ostream &out) const {
        bool first = true;
        for (const Field *field : m_payload) {
            if (!first) {
                out << ", ";
            }
            out << "{" << *field << "}";
            first = false;
        }

        return out;
    }
}

