//
// Created by Tom Fewster on 12/02/2016.
//

#include <iostream>
#include "Message.h"
#include "DataField.h"

namespace DCF {

    const uint16_t Message::_NO_FIELD;

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

    // from Encoder
    const size_t Message::encodeAddressing(MessageBuffer &buffer) noexcept {
        byte *b = buffer.allocate(sizeof(MsgAddressing));
        MsgAddressing *header = reinterpret_cast<MsgAddressing *>(b);

        size_t msgLength = sizeof(MsgAddressing);
        header->addressing_start = addressing_flag;
        header->flags = this->flags();
        header->reserved = 0;
        header->subject_length = static_cast<uint16_t>(strlen(this->subject()));
        buffer.append(reinterpret_cast<const byte *>(this->subject()), header->subject_length);
        msgLength += header->subject_length;

        return msgLength;
    }

    const size_t Message::encode(MessageBuffer &buffer) noexcept {
        size_t msgLength = 0;
        if (m_hasAddressing) {
            msgLength += encodeAddressing(buffer);
        }

        byte *b = buffer.allocate(sizeof(MsgHeader));
        msgLength += sizeof(MsgHeader);

        MsgHeader *header = reinterpret_cast<MsgHeader *>(b);
        header->header_start = body_flag;
        header->body_length = 0;
        header->field_count = this->size();

        for (const std::shared_ptr<Field> &field : m_payload) {
            header->body_length += field->encode(buffer);
        }

        return msgLength + header->body_length;
    }

    // from Decoder
    const size_t Message::decodeAddressing(const ByteStorage &buffer) noexcept {
        size_t read_offset = 0;

        const byte *bytes = nullptr;
        const size_t length = buffer.bytes(&bytes);
        if (bytes != nullptr && length != 0) {
            assert(bytes[0] == addressing_flag);
            if (length > sizeof(MsgAddressing)) {
                const MsgAddressing *addressing = reinterpret_cast<const MsgAddressing *>(bytes);
                read_offset += sizeof(MsgAddressing);

                m_flags = addressing->flags;
//                addressing->reserved;

                if (addressing->subject_length + read_offset <= length) {
                    const char *subject = reinterpret_cast<const char *>(&bytes[read_offset]);
                    memcpy(m_subject, subject, addressing->subject_length);
                    m_subject[addressing->subject_length] = '\0';
                    read_offset += addressing->subject_length;
                }
            }
        }

        return read_offset;
    }

    const size_t Message::decode(const ByteStorage &buffer) noexcept {
        size_t read_offset = 0;
        const byte *bytes = nullptr;
        const size_t length = buffer.bytes(&bytes);

        if (bytes[0] == addressing_flag) {
            read_offset += decodeAddressing(buffer);
            if (read_offset == 0) {
                // We didn't have enough data to read the header
                return 0;
            }
        }

        if (length - read_offset > sizeof(MsgHeader)) {
            bytes += read_offset;
            const MsgHeader *header = reinterpret_cast<const MsgHeader *>(bytes);
            read_offset += sizeof(MsgHeader);
            assert(bytes[0] == body_flag);

            if (header->body_length + read_offset <= buffer.length()) {
                for (size_t i = 0; i < header->field_count; i++) {
                    const byte *current_ptr = &bytes[read_offset];
                    const MsgField *f = reinterpret_cast<const MsgField *>(current_ptr);
                    std::shared_ptr<Field> field;
                    switch(f->type) {
                        case StorageType::string:
                        case StorageType::data:
                            field = std::make_shared<DataField>();
                            break;
                        case StorageType::message:
                            break;
                        default:
                            field = std::make_shared<ScalarField>();
                            break;
                    }

                    read_offset += field->decode(ByteStorage(current_ptr, buffer.length() - read_offset, true));
                    m_mapper[field->identifier()] = m_payload.size();
                    m_payload.emplace_back(field);
                    m_size++;

                    m_maxRef = std::max(m_maxRef, field->identifier());
                }
            }
        }

        return read_offset;
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