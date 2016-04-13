//
// Created by Tom Fewster on 12/04/2016.
//

#ifndef TFDCF_LARGEDATAFIELD_H
#define TFDCF_LARGEDATAFIELD_H

#include "DataField.h"

namespace DCF {
    template <typename Allocator> class LargeDataField final : public DataField {
    private:
        MutableByteStorage<byte, Allocator> m_storage;

    protected:
        virtual const bool isEqual(const Field &other) const noexcept override {
            if (typeid(other) == typeid(LargeDataField)) {
                const LargeDataField &f = static_cast<const LargeDataField &>(other);
                return m_type == f.m_type && m_storage == f.m_storage;
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

    public:
        LargeDataField(const Allocator allocator = Allocator()) : m_storage(32, allocator) {}

        const size_t size() const noexcept override { return m_storage.length(); }

        void set(const char *identifier, const char *value) noexcept override {
            setIdentifier(identifier);
            m_type = field_traits<const char *>::type;
            m_storage.setData(reinterpret_cast<const byte *>(value), strlen(value) + 1); // +1 for the NULL byte
        }

        void set(const char *identifier, const void *value, const size_t length) noexcept override {
            setIdentifier(identifier);
            m_type = field_traits<byte *>::type;
            m_storage.setData(reinterpret_cast<const byte *>(value), length);
        }

        const size_t get(const byte **data) const noexcept override {
            assert(m_type == StorageType::data);
            return m_storage.bytes(data);
        }

        const size_t get(const char **data) const noexcept override {
            assert(m_type == StorageType::string);
            const byte *bytes = nullptr;
            const size_t r =m_storage.bytes(&bytes);
            *data = reinterpret_cast<const char *>(bytes);
            return r;
        }

        template <typename T, typename = std::enable_if<field_traits<T>::value && std::is_pointer<T>::value>> const T get() const noexcept {
            return DataField::get<T>();
        }

        const size_t encode(MessageBuffer &buffer) const noexcept override {
            const byte *data = nullptr;
            const size_t data_len = m_storage.bytes(&data);
            return Field::encode(buffer, data, data_len);
        }

        const bool decode(const MessageBuffer::ByteStorageType &buffer) noexcept override {
            if (buffer.remainingReadLength() >= MsgField::size()) {
                m_type = static_cast<StorageType>(readScalar<MsgField::type>(buffer.readBytes()));
                buffer.advanceRead(sizeof(MsgField::type));

                const size_t identifier_length = readScalar<MsgField::identifier_length>(buffer.readBytes());
                buffer.advanceRead(sizeof(MsgField::identifier_length));

                const size_t size = readScalar<MsgField::data_length>(buffer.readBytes());
                buffer.advanceRead(sizeof(MsgField::data_length));

                if (buffer.remainingReadLength() >= identifier_length) {
                    std::copy(buffer.readBytes(), &buffer.readBytes()[identifier_length], m_identifier);
                    m_identifier[identifier_length] = '\0';
                    buffer.advanceRead(identifier_length);
                    if (buffer.remainingReadLength() >= size) {
                        m_storage.setData(buffer.readBytes(), size);
                        buffer.advanceRead(size);
                        return true;
                    }
                }
            }

            return false;
        }
    };
}

#endif //TFDCF_LARGEDATAFIELD_H
