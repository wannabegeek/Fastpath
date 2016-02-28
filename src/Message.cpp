//
// Created by Tom Fewster on 12/02/2016.
//

#include <iostream>
#include "Message.h"
#include "DataField.h"

namespace DCF {

    const uint16_t Message::_NO_FIELD;

    Message::Message(Message &&other) noexcept : m_size(other.m_size),
                                        m_flags(other.m_flags),
                                        m_hasAddressing(other.m_hasAddressing),
                                        m_payload(std::move(other.m_payload)),
                                        m_mapper(std::move(other.m_mapper)),
                                        m_maxRef(other.m_maxRef),
                                        m_keys(std::move(other.m_keys)) {
        std::swap(m_subject, other.m_subject);
        other.m_subject[0] = '\0';
//        other.clear();
    }

    const uint16_t Message::findIdentifierByName(const std::string &field, const size_t instance) const noexcept {
        auto it = m_keys.find(field);
        if (it != m_keys.end()) {
            if (instance < it->second.size()) {
                return it->second[instance];
            }
        }

        return _NO_FIELD;
    }

    const bool Message::refExists(const uint16_t &field) const noexcept {
        return m_mapper[field] != _NO_FIELD;
    }

    const uint16_t Message::createRefForString(const std::string &field) noexcept {
        const uint16_t ref = ++m_maxRef;
        auto it = m_keys.find(field);
        if (it == m_keys.end()) {
            m_keys.emplace(std::make_pair(field, KeyMappingsContainer::mapped_type()));
        }
        m_keys[field].push_back(ref);
        return ref;
    }

    void Message::clear() {
        m_size = 0;
        m_flags = -1;
        m_subject[0] = '\0';
        m_payload.clear();
        m_keys.clear();
        m_mapper.fill(_NO_FIELD);
        m_maxRef = 0;
    }

    void Message::addDataField(const uint16_t &field, const byte *value, const size_t size) {
        if (refExists(field)) {
            ThrowException(TF::Exception, "Ref already exists in message");
        }
        m_maxRef = std::max(m_maxRef, field);
        m_mapper[field] = m_payload.size();

        std::shared_ptr<DataField> e = std::make_shared<DataField>();
        e->set(field, value, size);
        m_payload.emplace_back(e);
        m_size++;
    }

    void Message::addMessageField(const uint16_t &field, const MessageType &msg) {
        if (refExists(field)) {
            ThrowException(TF::Exception, "Ref already exists in message");
        }
        m_maxRef = std::max(m_maxRef, field);
        m_mapper[field] = m_payload.size();

        msg->m_hasAddressing = false;
        std::shared_ptr<MessageField> e = std::make_shared<MessageField>();
        e->set(field, msg);
        m_payload.emplace_back(e);
        m_size++;
    }

    void Message::addDataField(const std::string &field, const byte *value, const size_t size) {
        const uint16_t ref = createRefForString(field);
        this->addDataField(ref, value, size);
    }

    void Message::addMessageField(const std::string &field, const MessageType &msg) {
        const uint16_t ref = createRefForString(field);
        this->addMessageField(ref, msg);
    }

    void Message::addMessageField(const MessageType &msg) {
        this->addMessageField(msg->subject(), msg);
    }

    bool Message::removeField(const uint16_t &field) {
        if (field != _NO_FIELD && m_mapper[field] != _NO_FIELD) {
            m_payload[m_mapper[field]] = nullptr;
            m_mapper[field] = _NO_FIELD;
            m_size--;

            return true;
        }
        return false;
    }

    bool Message::removeField(const std::string &field, const size_t instance) {
        const uint16_t ref = findIdentifierByName(field, instance);
        auto it = m_keys.find(field);
        if (it != m_keys.end()) {
            auto &list = it->second;
            if (instance < list.size()) {
                list.erase(list.begin() + instance);
                if (list.empty()) {
                    m_keys.erase(it);
                }
            }
        }

        return this->removeField(ref);
    }

    void Message::detach() noexcept {

    }

    const bool Message::operator==(const Message &other) const {
        return m_size == other.m_size
                && m_flags == other.m_flags
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

        size_t body_length = 0;

        for (const std::shared_ptr<Field> &field : m_payload) {
            body_length += field->encode(buffer);
        }

        const size_t total_len = msgLength + body_length;
        writeScalar(body_length_offset, static_cast<MsgHeader::body_length>(body_length));
        return total_len;
    }

    // from Decoder
    const bool Message::decodeAddressing(const ByteStorage &buffer, size_t &read_offset) noexcept {
        read_offset = 0;

        const byte *bytes = nullptr;
        const size_t length = buffer.bytes(&bytes);
        if (bytes != nullptr && length != 0) {
            MsgAddressing::addressing_start chk = readScalar<MsgAddressing::addressing_start>(bytes);
            bytes += sizeof(MsgAddressing::addressing_start);

            assert(chk == addressing_flag);
            if (length > MsgAddressing::size()) {
                read_offset += MsgAddressing::size();

                m_flags = readScalar<MsgAddressing::flags>(bytes);
                bytes += sizeof(MsgAddressing::flags);
//                addressing->reserved;
                bytes += sizeof(MsgAddressing::reserved);

                const size_t subject_length = readScalar<MsgAddressing::subject_length>(bytes);
                bytes += sizeof(MsgAddressing::subject_length);

                if (subject_length + read_offset <= length) {
                    const char *subject = reinterpret_cast<const char *>(bytes);
                    memcpy(m_subject, subject, subject_length);
                    m_subject[subject_length] = '\0';
                    read_offset += subject_length;
                    return false;
                }
            }
        }

        read_offset = 0;
        return false;
    }

    const bool Message::decode(const ByteStorage &buffer, size_t &read_offset) noexcept {
        const byte *bytes = nullptr;
        const size_t length = buffer.bytes(&bytes);
        const byte *start_ptr = bytes;

        bool success = false;

        if (bytes[0] == addressing_flag) {
            size_t len = 0;
            if (decodeAddressing(buffer, len)) {
                // We didn't have enough data to read the header
                return false;
            }

            bytes += len;
        }

        if (length - (bytes - start_ptr) > MsgHeader::size()) {

            MsgHeader::header_start chk = readScalar<MsgHeader::header_start>(bytes);
            bytes += sizeof(MsgHeader::header_start);
            assert(chk == body_flag);

            const MsgHeader::body_length body_length = readScalar<MsgHeader::body_length>(bytes);
            bytes += sizeof(MsgHeader::body_length);
            const MsgHeader::field_count field_count = readScalar<MsgHeader::field_count>(bytes);
            bytes += sizeof(MsgHeader::field_count);

            if (body_length + (bytes - start_ptr) <= buffer.length()) {
                for (size_t i = 0; i < field_count; i++) {

                    const StorageType type = static_cast<StorageType>(readScalar<MsgField::type>(bytes));
                    std::shared_ptr<Field> field;
                    switch(type) {
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

                    size_t len = 0;
                    if (!field->decode(ByteStorage(bytes, buffer.length() - (bytes - start_ptr), true), len)) {
                        return false;
                    }
                    bytes += len;
                    m_mapper[field->identifier()] = m_payload.size();
                    m_payload.emplace_back(field);
                    m_size++;

                    m_maxRef = std::max(m_maxRef, field->identifier());
                }

                read_offset = (bytes - start_ptr);
                success = true;
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