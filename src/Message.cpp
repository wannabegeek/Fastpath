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

    const bool Message::refExists(const short &field) const {
//            size_t t = m_payload.size();
        return m_payload[field] != nullptr;
    }

    void Message::addRawField(const void *data, const size_t size) {

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

}