//
// Created by Tom Fewster on 12/02/2016.
//

#include "Field.h"

namespace DCF {

    void Field::setValue(const int8_t &value) {
        m_data.i8 = value;
        m_type = is_valid_type<int8_t>::type;
        m_size = is_valid_type<int8_t>::size;
    }

    void Field::setValue(const uint8_t &value) {
        m_data.u8 = value;
        m_type = is_valid_type<uint8_t>::type;
        m_size = is_valid_type<uint8_t>::size;
    }

    void Field::setValue(const int16_t &value) {
        m_data.i16 = value;
        m_type = is_valid_type<int16_t>::type;
        m_size = is_valid_type<int16_t>::size;
    }

    void Field::setValue(const uint16_t &value) {
        m_data.u16 = value;
        m_type = is_valid_type<uint16_t>::type;
        m_size = is_valid_type<uint16_t>::size;
    }

    void Field::setValue(const int32_t &value) {
        m_data.i32 = value;
        m_type = is_valid_type<int32_t>::type;
        m_size = is_valid_type<int32_t>::size;
    }

    void Field::setValue(const uint32_t &value) {
        m_data.u32 = value;
        m_type = is_valid_type<uint32_t>::type;
        m_size = is_valid_type<uint32_t>::size;
    }

    void Field::setValue(const int64_t &value) {
        m_data.i64 = value;
        m_type = is_valid_type<int64_t>::type;
        m_size = is_valid_type<int64_t>::size;
    }

    void Field::setValue(const uint64_t &value) {
        m_data.u64 = value;
        m_type = is_valid_type<uint64_t>::type;
        m_size = is_valid_type<uint64_t>::size;
    }

    void Field::setValue(const float32_t &value) {
        m_data.f32 = value;
        m_type = is_valid_type<float32_t>::type;
        m_size = is_valid_type<float32_t>::size;
    }

    void Field::setValue(const float64_t &value) {
        m_data.f64 = value;
        m_type = is_valid_type<float64_t>::type;
        m_size = is_valid_type<float64_t>::size;
    }

    void Field::setValue(const bool &value) {
        m_data.boolean = value;
        m_type = is_valid_type<bool>::type;
        m_size = is_valid_type<bool>::size;
    }

    void Field::setValue(const Message &value) {
        // TODO:
    }

    void Field::setValue(const char *value) {
        m_storage.setData(reinterpret_cast<const byte *>(value), strlen(value) + 1);
        m_type = is_valid_type<const char *>::type;
        m_size = m_storage.length();
    }

    void Field::setValue(const std::string &value) {
        m_storage.setData(reinterpret_cast<const byte *>(value.c_str()), value.length());
        m_type = is_valid_type<std::string>::type;
        m_size = value.length();
    }

    void Field::setValue(const byte *data, const size_t size) {
        m_storage.setData(data, size);
        m_size = size;
        m_type = StorageType::data;
    }


    const bool Field::get(int8_t &value) const {
        if (m_type == is_valid_type<int8_t>::type) {
            value = m_data.i8;
            return true;
        }
        return false;
    }

    const bool Field::get(uint8_t &value) const {
        if (m_type == is_valid_type<uint8_t>::type) {
            value = m_data.u8;
            return true;
        }
        return false;
    }

    const bool Field::get(int16_t &value) const {
        if (m_type == is_valid_type<int16_t>::type) {
            value = m_data.i16;
            return true;
        }
        return false;
    }

    const bool Field::get(uint16_t &value) const {
        if (m_type == is_valid_type<uint16_t>::type) {
            value = m_data.u16;
            return true;
        }
        return false;
    }

    const bool Field::get(int32_t &value) const {
        if (m_type == is_valid_type<int32_t>::type) {
            value = m_data.i32;
            return true;
        }
        return false;
    }

    const bool Field::get(uint32_t &value) const {
        if (m_type == is_valid_type<uint32_t>::type) {
            value = m_data.u32;
            return true;
        }
        return false;
    }

    const bool Field::get(int64_t &value) const {
        if (m_type == is_valid_type<int64_t>::type) {
            value = m_data.i64;
            return true;
        }
        return false;
    }

    const bool Field::get(uint64_t &value) const {
        if (m_type == is_valid_type<uint64_t>::type) {
            value = m_data.u64;
            return true;
        }
        return false;
    }

    const bool Field::get(float32_t &value) const {
        if (m_type == is_valid_type<float32_t>::type) {
            value = m_data.f32;
            return true;
        }
        return false;
    }

    const bool Field::get(float64_t &value) const {
        if (m_type == is_valid_type<float64_t>::type) {
            value = m_data.f64;
            return true;
        }
        return false;
    }

    const bool Field::get(bool &value) const {
        if (m_type == is_valid_type<bool>::type) {
            value = m_data.boolean;
            return true;
        }
        return false;
    }

    const bool Field::get(const char **value) const {
        const byte *data = nullptr;
        if (m_type == StorageType::string && m_storage.bytes(&data) != 0) {
            *value = reinterpret_cast<const char *>(data);
            return true;
        }

        return false;
    }

    const bool Field::get(std::string &value) const {
        const byte *data = nullptr;
        if (m_type == StorageType::string && m_storage.bytes(&data) != 0) {
            value = std::string(reinterpret_cast<const char *>(data), m_storage.length());
            return true;
        }

        return false;
    }

    const bool Field::get(const byte **value, size_t &size) const {
        const byte *data = nullptr;
        if (m_type == StorageType::data && (size = m_storage.bytes(&data)) != 0) {
            *value = data;
            return true;
        }

        return false;
    }
}