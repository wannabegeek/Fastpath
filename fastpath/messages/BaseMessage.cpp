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

#include "fastpath/messages/Message.h"

#include "fastpath/utils/logger.h"
#include "fastpath/messages/DataField.h"
#include "fastpath/messages/DateTimeField.h"
#include "fastpath/messages/MessageField.h"

#include <iostream>

namespace fp {

    BaseMessage::BaseMessage(BaseMessage &&other) noexcept : m_payload(std::move(other.m_payload)),
                                        m_keys(std::move(other.m_keys)), m_field_allocator(std::move(other.m_field_allocator)) {
    }

    BaseMessage::~BaseMessage() {
//        this->clear();
    }

    void BaseMessage::clear() {
        for (Field *field : m_payload) {
            this->destroyField(field);
        }
        m_payload.clear();
        m_keys.clear();
    }

    bool BaseMessage::addDataField(const char *field, const byte *value, const size_t size) {
        DataField *e = this->createDataField(size, field, value, size);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool BaseMessage::addDataField(const char *field, const char *value) {
        DataField *e = this->createDataField(strlen(value) + 1, field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool BaseMessage::addMessageField(const char *field, BaseMessage &&msg) {
        MessageField *e = this->createMessageField(field, std::forward<BaseMessage>(msg));
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool BaseMessage::addDateTimeField(const char *field, const std::chrono::time_point<std::chrono::system_clock> &time) {
        DateTimeField *e = this->createDateTimeField(field, time);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool BaseMessage::addDateTimeField(const char *field, const std::chrono::microseconds &time) {
        DateTimeField *e = this->createDateTimeField(field, time);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool BaseMessage::addDateTimeField(const char *field, const uint64_t seconds, const uint64_t microseconds) {
        DateTimeField *e = this->createDateTimeField(field, seconds, microseconds);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            this->destroyField(e);
        }
        return result.second;
    }

    bool BaseMessage::removeField(const char* field) {
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

    void BaseMessage::detach() noexcept {
    }

    const bool BaseMessage::operator==(const BaseMessage &other) const {
        return std::equal(m_payload.begin(), m_payload.end(), other.m_payload.begin(), [](const PayloadContainer::value_type& item1, const PayloadContainer::value_type& item2) -> bool {
                        return *item1 == *item2;
                });
    }

    const size_t BaseMessage::encode(MessageBuffer &buffer) const noexcept {
        size_t msgLength = 0;

        byte *b = buffer.allocate(MsgHeader::size());
        msgLength += MsgHeader::size();

        b = writeScalar(b, static_cast<MsgHeader::header_start>(body_flag));
        b = writeScalar(b, static_cast<MsgHeader::field_count>(this->size()));


        for (const Field *field : m_payload) {
            msgLength += field->encode(buffer);
        }

        return msgLength;
    }

    const bool BaseMessage::decode(const MessageBuffer::ByteStorageType &buffer) throw (fp::exception) {
        bool success = false;
        assert(buffer.length() > 0);

        if (buffer.remainingReadLength() > MsgHeader::size()) {
            MsgHeader::header_start chk = readScalar<MsgHeader::header_start>(buffer.readBytes());
            buffer.advanceRead(sizeof(MsgHeader::header_start));
            if (chk != body_flag) {
                throw fp::exception("Received corrupt message - incorrect body marker");
            }

            const MsgHeader::field_count field_count = readScalar<MsgHeader::field_count>(buffer.readBytes());
            buffer.advanceRead(sizeof(MsgHeader::field_count));

            for (size_t i = 0; i < field_count; i++) {
                MsgField::type type = unknown;
                MsgField::identifier_length identifier_size = 0;
                MsgField::data_length data_size = 0;
                Field::peek_field_header(buffer, type, identifier_size, data_size);

                Field *field = nullptr;
                switch (static_cast<storage_type>(type)) {
                    case storage_type::string:
                    case storage_type::data:
                        field = this->createDataField(data_size, buffer);
                        break;
                    case storage_type::date_time:
                        field = this->createDateTimeField(buffer);
                        break;
                    case storage_type::message:
                        field = this->createMessageField(buffer);
                        break;
                    default:
                        field = this->createScalarField(buffer);
                        break;
                }

                m_keys.insert(std::make_pair(field->identifier(), m_payload.size()));
                m_payload.emplace_back(field);
                success = true;
            }
        }
        return success;
    }

    const DataStorageType BaseMessage::getStorageType(const storage_type type) {
        DataStorageType r = scalar_t;
        switch (type) {
            case storage_type::data:
            case storage_type::string:
                r = data_t;
                break;
            case storage_type::message:
                r = message_t;
                break;
            default:
                break;
        }
        return r;
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