//
// Created by Tom Fewster on 12/02/2016.
//

#include "Element.h"

namespace DCF {
    template<> void Element::setValue(const std::string &value) {
        m_value = value.c_str();
        m_type = is_valid_type<std::string>::type;
    }

    void Element::setValue(const char *value) {
        m_value = value;
        m_type = is_valid_type<const char *>::type;
    }

    void Element::setValue(const void *data, const size_t size) {
        m_value = data;
        m_size = size;
        m_type = StorageType::data;
    }

    template <> const bool Element::get(std::string &value) const {
        try {
            value = std::string(boost::any_cast<const char *>(m_value));
        } catch(const boost::bad_any_cast &) {
            return false;
        }

        return true;

    }


}