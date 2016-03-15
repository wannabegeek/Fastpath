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
                                        m_keys(std::move(other.m_keys)) {
    }

    void BaseMessage::clear() {
        m_payload.clear();
        m_keys.clear();
    }

    bool BaseMessage::addDataField(const char *field, const byte *value, const size_t size) {
        std::shared_ptr<DataField> e = std::make_shared<DataField>();
        e->set(field, value, size);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        }
        return result.second;
    }

    bool BaseMessage::addDataField(const char *field, const char *value) {
        std::shared_ptr<DataField> e = std::make_shared<DataField>();
        e->set(field, value);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        }
        return result.second;
    }

    bool BaseMessage::addMessageField(const char *field, const BaseMessage *msg) {
        std::shared_ptr<MessageField> e = std::make_shared<MessageField>();
        e->set(field, msg);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        }
        return result.second;
    }

    bool BaseMessage::addDateTimeField(const char *field, const std::chrono::time_point<std::chrono::system_clock> &time) {
        std::shared_ptr<DateTimeField> e = std::make_shared<DateTimeField>();
        e->set(field, time);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
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
//        b += sizeof(MsgHeader::field_count);

        for (const std::shared_ptr<Field> &field : m_payload) {
            msgLength += field->encode(buffer);
        }

        return msgLength;
    }

    const bool BaseMessage::decode(const ByteStorage &buffer) {
        bool success = false;
        const size_t length = buffer.length();

        if (length != 0) {
            if (buffer.remainingReadLength() > MsgHeader::size()) {

                MsgHeader::header_start chk = readScalar<MsgHeader::header_start>(buffer.readBytes());
                buffer.advanceRead(sizeof(MsgHeader::header_start));
                assert(chk == body_flag);

                const MsgHeader::field_count field_count = readScalar<MsgHeader::field_count>(buffer.readBytes());
                buffer.advanceRead(sizeof(MsgHeader::field_count));

                for (size_t i = 0; i < field_count; i++) {
                    const StorageType type = static_cast<StorageType>(readScalar<MsgField::type>(buffer.readBytes()));
                    std::shared_ptr<Field> field;
                    switch (type) {
                        case StorageType::string:
                        case StorageType::data:
                            field = std::make_shared<DataField>();
                            break;
                        case StorageType::date_time:
                            field = std::make_shared<DateTimeField>();
                            break;
                        case StorageType::message:
                            field = std::make_shared<MessageField>();
                            break;
                        default:
                            field = std::make_shared<ScalarField>();
                            break;
                    }

                    if (!field->decode(buffer)) {
                        return false;
                    }
                    m_payload.emplace_back(field);
                    m_keys.insert(std::make_pair(field->identifier(), m_payload.size()));

                    success = true;
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
        for (const std::shared_ptr<Field> &field : m_payload) {
            if (!first) {
                out << ", ";
            }
            out << "{" << *field.get() << "}";
            first = false;
        }

        return out;
    }
}