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
    public:
        const StorageType type() const noexcept override { return m_type; }
        const size_t size() const noexcept override { return m_storage.length(); }

        void set(const uint16_t identifier, const char *value) {
            m_identifier = identifier;
            m_type = field_traits<const char *>::type;
            m_storage.setData(reinterpret_cast<const byte *>(value), strlen(value) + 1); // +1 for the NULL byte
        }

        template <typename T, typename = std::enable_if<field_traits<T>::value && std::is_pointer<T>::value>> void set(const uint16_t identifier, const T *value, const size_t length) {
            m_identifier = identifier;
            m_type = field_traits<T *>::type;
            m_storage.setData(reinterpret_cast<const byte *>(value), length);
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

        const bool operator==(const DataField &other) const {
            return m_identifier == other.m_identifier
                    && m_type == other.m_type
                    && m_storage == other.m_storage;
        }

        const size_t encode(MessageBuffer &buffer) noexcept override {
            byte *b = buffer.allocate(MsgField::size());

            b = writeScalar(b, static_cast<MsgField::type>(m_type));
            b = writeScalar(b, static_cast<MsgField::identifier>(m_identifier));

            const byte *data = nullptr;
            const size_t len = m_storage.bytes(&data);
            b = writeScalar(b, static_cast<MsgField::data_length>(len));
            buffer.append(data, len);

            return MsgField::size() + len;
        }

        const bool decode(const ByteStorage &buffer, size_t &read_offset) noexcept override {
            if (buffer.length() >= MsgField::size()) {
                const byte *data = nullptr;
                buffer.bytes(&data);

                m_type = static_cast<StorageType>(readScalar<MsgField::type>(data));
                data += sizeof(MsgField::type);

                m_identifier = readScalar<MsgField::identifier>(data);
                data += sizeof(MsgField::identifier);

                const size_t size = readScalar<MsgField::data_length>(data);
                data += sizeof(MsgField::data_length);

                if (buffer.length() >= MsgField::size() + size) {
                    m_storage.setData(data, size);

                    read_offset = MsgField::size() + size;
                    return true;
                }
            }

            return false;
        }

        virtual std::ostream& output(std::ostream& out) const override {
            switch (m_type) {
                case StorageType::string: {
                    const byte *data;
                    const size_t size = m_storage.bytes(&data);
                    out << m_identifier << ":string=" << std::string(reinterpret_cast<const char *>(data), size - 1); // -1 for NULL
                    break;
                }
                case StorageType::data: {
                    out << m_identifier << ":opaque=" << "[data of " << m_storage.length() << " bytes]";
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
