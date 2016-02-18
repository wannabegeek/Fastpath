//
// Created by Tom Fewster on 12/02/2016.
//

#include "Message.h"

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

    void Message::addField(const uint16_t &field, const byte *value, const size_t size) {
        if (refExists(field)) {
            ThrowException(TF::Exception, "Ref already exists in message");
        }
        m_maxRef = std::max(m_maxRef, field);
        m_mapper[field] = m_payload.size();

        std::shared_ptr<Field> e = std::make_shared<Field>();
        e->setValue(value, size);
        m_payload.emplace_back(e);
        m_size++;
    }

    void Message::addField(const std::string &field, const byte *value, const size_t size) {
        const uint16_t ref = createRefForString(field);
        this->addField(ref, value, size);
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
    void Message::encode(MessageBuffer &buffer) noexcept {
        byte *b = buffer.allocate(sizeof(MsgHeader));

        MsgHeader *header = reinterpret_cast<MsgHeader *>(b);
        header->msg_length = 256;
        header->flags = this->flags();
        header->field_count = this->size();
        header->subject_length = static_cast<uint16_t>(strlen(this->subject()));
        buffer.append(reinterpret_cast<const byte *>(this->subject()), header->subject_length);

        for (const std::shared_ptr<Field> &field : m_payload) {
            field->encode(buffer);
        }
        return;
    }

    // from Decoder
    void Message::decode(MessageBuffer &buffer) noexcept {

    }

}