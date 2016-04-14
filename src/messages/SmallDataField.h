//
// Created by Tom Fewster on 12/04/2016.
//

#ifndef TFDCF_SMALLDATAFIELD_H
#define TFDCF_SMALLDATAFIELD_H

#include "DataField.h"
#include <cstring>

namespace DCF {
    class SmallDataField final : public DataField {
    public:
        static constexpr std::size_t max_size = 256;
    private:
        byte m_storage[max_size];

    protected:
        virtual const bool isEqual(const Field &other) const noexcept override {
            if (typeid(other) == typeid(SmallDataField)) {
                const SmallDataField &f = static_cast<const SmallDataField &>(other);
                return std::memcmp(m_storage, f.m_storage, m_data_length) == 0;
            }
            return false;
        }

        virtual std::ostream& output(std::ostream& out) const override {
            switch (m_type) {
                case StorageType::string: {
                    out << m_identifier << ":string=" << std::string(reinterpret_cast<const char *>(m_storage), m_data_length - 1); // -1 for NULL
                    break;
                }
                case StorageType::data: {
                    out << m_identifier << ":opaque=" << "[data of " << m_data_length << " bytes]";
                    break;
                }
                default:
                    // we can't handle any other message type
                    break;
            }

            return out;
        }

    public:
        SmallDataField(const char *identifier, const char *value) noexcept : DataField(identifier, StorageType::string, strlen(value) + 1) {
            std::strncpy(reinterpret_cast<char *>(m_storage), value, m_data_length);
        }

        SmallDataField(const char *identifier, const byte *value, const std::size_t length) noexcept : DataField(identifier, StorageType::data, length) {
            std::memcpy(m_storage, value, m_data_length);
        }

        SmallDataField(const MessageBuffer::ByteStorageType &buffer) : DataField(buffer) {
            std::memcpy(m_storage, buffer.readBytes(), m_data_length);
            buffer.advanceRead(m_data_length);
        }

        const size_t get(const byte **data) const noexcept override {
            assert(m_type == StorageType::data);
            *data = m_storage;
            return m_data_length;
        }

        const size_t get(const char **data) const noexcept override {
            assert(m_type == StorageType::string);
            *data = reinterpret_cast<const char *>(m_storage);
            return m_data_length;
        }

        template <typename T, typename = std::enable_if<field_traits<T>::value && std::is_pointer<T>::value>> const T get() const noexcept {
            return DataField::get<T>();
        }

        const size_t encode(MessageBuffer &buffer) const noexcept override {
            return Field::encode(buffer, m_storage, m_data_length);
        }

        const bool decode(const MessageBuffer::ByteStorageType &buffer) noexcept override {
            return false;
        }
    };
}

#endif //TFDCF_SMALLDATAFIELD_H
