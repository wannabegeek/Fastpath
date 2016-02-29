//
// Created by Tom Fewster on 12/02/2016.
//

#include <iostream>
#include "Message.h"
#include "DataField.h"

namespace DCF {

    Message::Message(Message &&other) noexcept : m_flags(other.m_flags),
                                        m_hasAddressing(other.m_hasAddressing),
                                        m_payload(std::move(other.m_payload)),
                                        m_keys(std::move(other.m_keys)) {
        std::swap(m_subject, other.m_subject);
        other.m_subject[0] = '\0';
//        other.clear();
    }

    void Message::clear() {
        m_flags = -1;
        m_subject[0] = '\0';
        m_payload.clear();
        m_keys.clear();
    }

    bool Message::addDataField(const char *field, const byte *value, const size_t size) {
        std::shared_ptr<DataField> e = std::make_shared<DataField>();
        e->set(field, value, size);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        }
        return result.second;
    }

    bool Message::addMessageField(const char *field, const MessageType &msg) {
        msg->m_hasAddressing = false;
        std::shared_ptr<MessageField> e = std::make_shared<MessageField>();
        e->set(field, msg);
        auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
        if (result.second) {
            m_payload.emplace_back(e);
        }
        return result.second;
    }

    bool Message::removeField(const char* field) {
        if (field != nullptr) {
            auto index = m_keys.find(field);
            m_keys.erase(field);
            m_payload.erase(m_payload.begin() + index->second);
            return true;
        }
        return false;
    }

    void Message::detach() noexcept {
    }

    const bool Message::operator==(const Message &other) const {
        return m_flags == other.m_flags
                && strncmp(m_subject, other.m_subject, std::numeric_limits<uint16_t>::max()) == 0;

        // TODO: this isn't complete
//                && std::equal(m_payload.begin(), m_payload.end(), other.m_payload.begin(), [](const PayloadContainer::value_type& item1, const PayloadContainer::value_type& item2) -> bool {
//            Field &f1 = *item1;
//            Field &f2 = *item2;
//                        return f1 == f2;
//                    });
    }

    // from Encoder
    const size_t Message::encodeAddressing(MessageBuffer &buffer) noexcept {
        byte *b = buffer.allocate(MsgAddressing::size());
        size_t msgLength = MsgAddressing::size();

        b = writeScalar(b, static_cast<MsgAddressing::addressing_start>(addressing_flag));
        b = writeScalar(b, static_cast<MsgAddressing::flags>(this->flags()));
        b += sizeof(MsgAddressing::reserved);
        const size_t subject_length = strlen(this->subject());
        b = writeScalar(b, static_cast<MsgAddressing::subject_length>(subject_length));

        buffer.append(reinterpret_cast<const byte *>(this->subject()), subject_length);
        msgLength += subject_length;

        return msgLength;
    }

    const size_t Message::encode(MessageBuffer &buffer) noexcept {
        size_t msgLength = 0;
        if (m_hasAddressing) {
            msgLength += encodeAddressing(buffer);
        }

        byte *b = buffer.allocate(MsgHeader::size());
        msgLength += MsgHeader::size();

        b = writeScalar(b, static_cast<MsgHeader::header_start>(body_flag));
        byte *body_length_offset = b;
        b += sizeof(MsgHeader::body_length);
        b = writeScalar(b, static_cast<MsgHeader::field_count>(this->size()));
//        b += sizeof(MsgHeader::field_count);

        size_t body_length = 0;

        for (const std::shared_ptr<Field> &field : m_payload) {
            body_length += field->encode(buffer);
        }

        const size_t total_len = msgLength + body_length;
        writeScalar(body_length_offset, static_cast<MsgHeader::body_length>(body_length));
        return total_len;
    }

    // from Decoder
    const bool Message::decodeAddressing(const ByteStorage &buffer) noexcept {
        if (buffer.remainingReadLength() >= MsgAddressing::size()) {
            MsgAddressing::addressing_start chk = readScalar<MsgAddressing::addressing_start>(buffer.readBytes());
            buffer.advanceRead(sizeof(MsgAddressing::addressing_start));
            assert(chk == addressing_flag);

            m_flags = readScalar<MsgAddressing::flags>(buffer.readBytes());
            buffer.advanceRead(sizeof(MsgAddressing::flags));

            buffer.advanceRead(sizeof(MsgAddressing::reserved));

            const size_t subject_length = readScalar<MsgAddressing::subject_length>(buffer.readBytes());
            buffer.advanceRead(sizeof(MsgAddressing::subject_length));

            if (buffer.remainingReadLength() >= subject_length) {
                const char *subject = reinterpret_cast<const char *>(buffer.readBytes());
                memcpy(m_subject, subject, subject_length);
                m_subject[subject_length] = '\0';
                buffer.advanceRead(subject_length);
                return false;
            }
        }

        return false;
    }

    const bool Message::decode(const ByteStorage &buffer) noexcept {
        bool success = false;
        const size_t length = buffer.length();

        if (length != 0) {

            const byte *bytes = buffer.readBytes();

            if (bytes[0] == addressing_flag) {
                if (decodeAddressing(buffer)) {
                    // We didn't have enough data to read the header
                    buffer.resetRead();
                    return false;
                }
            }

            if (buffer.remainingReadLength() > MsgHeader::size()) {

                MsgHeader::header_start chk = readScalar<MsgHeader::header_start>(buffer.readBytes());
                buffer.advanceRead(sizeof(MsgHeader::header_start));
                assert(chk == body_flag);

                const MsgHeader::body_length body_length = readScalar<MsgHeader::body_length>(buffer.readBytes());
                buffer.advanceRead(sizeof(MsgHeader::body_length));
                const MsgHeader::field_count field_count = readScalar<MsgHeader::field_count>(buffer.readBytes());
                buffer.advanceRead(sizeof(MsgHeader::field_count));

                if (buffer.remainingReadLength() >= body_length) {
                    for (size_t i = 0; i < field_count; i++) {

                        const StorageType type = static_cast<StorageType>(readScalar<MsgField::type>(buffer.readBytes()));
                        std::shared_ptr<Field> field;
                        switch (type) {
                            case StorageType::string:
                            case StorageType::data:
                                field = std::make_shared<DataField>();
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
                    }

                    success = true;
                }
            }
        }
        return success;
    }

    const DataStorageType Message::getStorageType(const StorageType type) {
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
}