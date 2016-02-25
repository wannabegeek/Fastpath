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

        void encode(MessageBuffer &buffer) noexcept override {
            byte *b = buffer.allocate(sizeof(Field));
            MsgField *field = reinterpret_cast<MsgField *>(b);
            field->identifier = m_identifier;
            field->type = m_type;
            const byte *data = nullptr;
            field->data_length = m_storage.bytes(&data);
            buffer.append(data, field->data_length);
        }

        const size_t decode(const ByteStorage &buffer) noexcept override {
            assert(buffer.length() > FieldHeaderSize());
            const byte *data = nullptr;
            buffer.bytes(&data);
            const MsgField *field = reinterpret_cast<const MsgField *>(data);

            m_identifier = field->identifier;
            m_type = static_cast<StorageType>(field->type);
            const size_t size = field->data_length;

            assert(buffer.length() > FieldHeaderSize() + size);
            m_storage.setData(&data[FieldHeaderSize()], size);

            return FieldHeaderSize() + size;
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
