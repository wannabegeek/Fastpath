//
// Created by Tom Fewster on 12/02/2016.
//

#include "Message.h"

namespace DCF {

    const uint16_t Message::findIdentifierByName(const std::string &field, const size_t instance) const {
        auto it = m_keys.find(field);
        if (it != m_keys.end()) {
            if (instance < it->second.size()) {
                return it->second[instance];
            }
        }

        return _NO_FIELD;
    }

    const bool Message::refExists(const uint16_t &field) const {
//            size_t t = m_payload.size();
        return m_payload[field] != nullptr;
    }

    const uint16_t Message::createRefForString(const std::string &field) {
        const uint16_t ref = ++m_maxRef;
        auto it = m_keys.find(field);
        if (it == m_keys.end()) {
            m_keys.emplace(std::make_pair(field, KeyMappingsContainer::mapped_type()));
        }
        m_keys[field].push_back(ref);
        return ref;
    }

    void Message::addField(const uint16_t &field, const byte *value, const size_t size) {
        m_maxRef = std::max(m_maxRef, field);
        if (refExists(field)) {
            ThrowException(TF::Exception, "Ref already exists in message");
        }
        std::shared_ptr<Field> e = std::make_shared<Field>();
        e->setValue(value, size);
        m_payload[field] = e;
    }

    void Message::addField(const std::string &field, const byte *value, const size_t size) {
        const uint16_t ref = createRefForString(field);
        this->addField(ref, value, size);
    }

    bool Message::removeField(const uint16_t &field) {
        if (field != _NO_FIELD && field <= m_maxRef) {
            m_payload[field] = nullptr;
            return true;
        }
        return false;
    }

    bool Message::removeField(const std::string &field, const size_t instance) {
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

        return this->removeField(findIdentifierByName(field, instance));
    }

    void Message::detach() {

    }
}