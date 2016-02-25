//
// Created by Tom Fewster on 24/02/2016.
//

#ifndef TFDCF_DATAFIELD_H
#define TFDCF_DATAFIELD_H

#include "Field.h"

namespace DCF {
    class DataField : public Field {
    private:
        MutableByteStorage m_storage;
        StorageType m_type;
        size_t m_size;
    public:
        const StorageType type() const noexcept override { return m_type; }
        const size_t size() const noexcept override { return m_size; }

        void set(const uint16_t identifier, const char *value) {
            m_identifier = identifier;
            m_type = field_traits<const char *>::type;
            m_size = strlen(value) + 1; // +1 for the NULL byte
            m_storage.setData(reinterpret_cast<const byte *>(value), m_size);
        }

        template <typename T, typename = std::enable_if<field_traits<T>::value && std::is_pointer<T>::value>> void set(const uint16_t identifier, const T *value, const size_t length) {
            m_identifier = identifier;
            m_type = field_traits<T *>::type;
            m_size = length;
            m_storage.setData(reinterpret_cast<const byte *>(value), m_size);
        }

        template <typename T, typename = std::enable_if<field_traits<T>::value && std::is_pointer<T>::value>> const T get() {
            assert(m_type == field_traits<T>::type);
            const byte *bytes = nullptr;
            m_storage.bytes(&bytes);
            return reinterpret_cast<T>(bytes);
        }

        const size_t get(const byte **data) {
            assert(m_type == StorageType::data);
            return m_storage.bytes(data);
        }

        const size_t get(const char **data) {
            assert(m_type == StorageType::string);
            const byte *bytes = nullptr;
            const size_t r =m_storage.bytes(&bytes);
            *data = reinterpret_cast<const char *>(bytes);
            return r;
        }

        void encode(MessageBuffer &buffer) noexcept override {
        }

        const size_t decode(const ByteStorage &buffer) noexcept override {
            return 0;
        }

        friend std::ostream &operator<<(std::ostream &out, const DataField &field) {
            switch (field.type()) {
                case StorageType::string: {
                    const byte *data;
                    const size_t size = field.m_storage.bytes(&data);
                    out << field.m_identifier << ":string=" << std::string(reinterpret_cast<const char *>(data), size - 1); // -1 for NULL
                    break;
                }
                case StorageType::data: {
                    out << field.m_identifier << ":opaque=" << "[data of " << field.m_storage.length() << " bytes]";
                    break;
                }
                default:
                    // we can't handle any other message type
                    break;
            }

            return out;
        }
    };
}

#endif //TFDCF_DATAFIELD_H
