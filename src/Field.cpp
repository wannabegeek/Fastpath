//
// Created by Tom Fewster on 12/02/2016.
//

#include "Field.h"

namespace DCF {


    void Field::setValue(const char *value) {
        m_storage.storeData(reinterpret_cast<const byte *>(value), strlen(value));
        m_type = is_valid_type<const char *>::type;
        m_size = m_storage.size();
    }

    template <> void Field::setValue(const std::string &value) {
        m_storage.storeData(reinterpret_cast<const byte *>(value.c_str()), value.length());
        m_type = is_valid_type<std::string>::type;
        m_size = value.length();
    }

    void Field::setValue(const byte *data, const size_t size) {
        m_storage.storeData(data, size);
        m_size = size;
        m_type = StorageType::data;
    }

    const bool Field::get(const char **value) const {
        const byte *data = nullptr;
        if (m_type == StorageType::string && m_storage.retreiveData(&data) != 0) {
            *value = reinterpret_cast<const char *>(data);
            return true;
        }

        return false;
    }

    template <> const bool Field::get(std::string &value) const {
        const byte *data = nullptr;
        if (m_type == StorageType::string && m_storage.retreiveData(&data) != 0) {
            value = std::string(reinterpret_cast<const char *>(data), m_storage.size());
            return true;
        }

        return false;
    }

    const bool Field::get(const byte **value, size_t &size) const {
        const byte *data = nullptr;
        if (m_type == StorageType::data && (size = m_storage.retreiveData(&data)) != 0) {
            *value = data;
            return true;
        }

        return false;
    }
}