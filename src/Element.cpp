//
// Created by Tom Fewster on 12/02/2016.
//

#include "Element.h"

namespace DCF {

    void Element::setValue(const char *value) {
        m_value = std::string(value);
        m_type = is_valid_type<const char *>::type;
        m_size = strlen(value);
    }

    template <> void Element::setValue(const std::string &value) {
        m_value = value;
        m_type = is_valid_type<std::string>::type;
        m_size = value.length();
    }

    void Element::setValue(const void *data, const size_t size) {
        m_value = data;
        m_size = size;
        m_type = StorageType::data;
    }

    const bool Element::get(const char **value) const {
        try {
            const std::string s = boost::any_cast<std::string>(m_value);
            *value = s.c_str();
        } catch(const boost::bad_any_cast &) {
            return false;
        }

        return true;

    }
}