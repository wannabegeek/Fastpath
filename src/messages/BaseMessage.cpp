//
// Created by Tom Fewster on 12/02/2016.
//

#include <iostream>
#include <utils/logger.h>
#include "Message.h"
#include "DataField.h"
#include "DateTimeField.h"
#include "MessageField.h"

namespace DCF {

    BaseMessage::BaseMessage(BaseMessage &&other) noexcept : m_payload(std::move(other.m_payload)),
                                        m_keys(std::move(other.m_keys)), m_allocator(std::move(other.m_allocator)) {
        m_payload.clear();
        m_keys.clear();
    }

    BaseMessage::~BaseMessage() {
        this->clear();
    }

    void BaseMessage::clear() {
//        size_t size = 0;
        for (Field *field : m_payload) {
            delete field;
//            switch (field->type()) {
//                case StorageType::string:
//                case StorageType::data:
//                    size = sizeof(DataField);
//                    break;
//                case StorageType::date_time:
//                    size = sizeof(DateTimeField);
//                    break;
//                case StorageType::message:
//                    size = sizeof(MessageField);
//                    break;
//                default:
//                    size = sizeof(ScalarField);
//                    break;
//            }
//
//            field->~Field();
//            field_allocator_traits::deallocate(m_field_allocator, reinterpret_cast<char *>(field), size);
        }
        m_payload.clear();
        m_keys.clear();
    }

    bool BaseMessage::addDataField(const char *field, const byte *value, const size_t size) {
        DataFieldType *e = new DataFieldType(m_allocator);
        e->set(field, value, size);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            delete e;
        }
        return result.second;
    }

    bool BaseMessage::addDataField(const char *field, const char *value) {
        DataFieldType *e = new DataFieldType(m_allocator);
        e->set(field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            delete e;
        }
        return result.second;
    }

    bool BaseMessage::addMessageField(const char *field, const BaseMessage *msg) {
        MessageField *e = new MessageField();
        e->set(field, msg);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            delete e;
        }
        return result.second;
    }

    bool BaseMessage::addDateTimeField(const char *field, const std::chrono::time_point<std::chrono::system_clock> &time) {
        DateTimeFieldType *e = new DateTimeFieldType(m_allocator);
        e->set(field, time);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        } else {
            delete e;
        }
        return result.second;
    }

    bool BaseMessage::removeField(const char* field) {
        if (field != nullptr) {
            auto index = m_keys.find(field);
            if (index != m_keys.end()) {
                m_keys.erase(field);
                delete field;
                auto it = m_payload.begin();
                std::advance(it, index->second);
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
                        return item1 == item2;
                });
    }

    const size_t BaseMessage::encode(MessageBuffer &buffer) const noexcept {
        size_t msgLength = 0;

        byte *b = buffer.allocate(MsgHeader::size());
        msgLength += MsgHeader::size();

        b = writeScalar(b, static_cast<MsgHeader::header_start>(body_flag));
        b = writeScalar(b, static_cast<MsgHeader::field_count>(this->size()));
//        b += sizeof(MsgHeader::field_count);

        for (const Field *field : m_payload) {
            msgLength += field->encode(buffer);
        }

        return msgLength;
    }

    const bool BaseMessage::decode(const MessageBuffer::ByteStorageType &buffer) {
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
                const StorageType type = static_cast<StorageType>(readScalar<MsgField::type>(buffer.readBytes()));
                Field *field = nullptr;
                switch (type) {
                    case StorageType::string:
                    case StorageType::data:
                        field = new DataFieldType(m_allocator);
                        break;
                    case StorageType::date_time:
                        field = new DateTimeFieldType(m_allocator);
                        break;
                    case StorageType::message:
                        field = new MessageField();
                        break;
                    default:
                        field = new ScalarField();
                        break;
                }

                if (field->decode(buffer)) {
                    m_keys.insert(std::make_pair(field->identifier(), m_payload.size()));
                    m_payload.emplace_back(field);
                    success = true;
                } else {
                    success = false;
                    break;
                }
            }
        }
        return success;
    }

    const DataStorageType BaseMessage::getStorageType(const StorageType type) {
        DataStorageType r = scalar_t;
        switch (type) {
            case StorageType::data:
            case StorageType::string:
                r = data_t;
                break;
            case StorageType::message:
                r = message_t;
                break;
            default:
                break;
        }
        return r;
    }

    std::ostream &BaseMessage::output(std::ostream &out) const {
        bool first = true;
        for (const Field *field : m_payload) {
            if (!first) {
                out << ", ";
            }
            out << "{" << field << "}";
            first = false;
        }

        return out;
    }
}