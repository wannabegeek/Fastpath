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
        std::size_t m_length;

    protected:
        virtual const bool isEqual(const Field &other) const noexcept override {
            if (typeid(other) == typeid(SmallDataField)) {
                const SmallDataField &f = static_cast<const SmallDataField &>(other);
                return m_type == f.m_type && m_length == f.m_length && std::memcmp(m_storage, f.m_storage, m_length) == 0;
            }
            return false;
        }

        virtual std::ostream& output(std::ostream& out) const override {
            switch (m_type) {
                case StorageType::string: {
                    out << m_identifier << ":string=" << std::string(reinterpret_cast<const char *>(m_storage), m_length - 1); // -1 for NULL
                    break;
                }
                case StorageType::data: {
                    out << m_identifier << ":opaque=" << "[data of " << m_length << " bytes]";
                    break;
                }
                default:
                    // we can't handle any other message type
                    break;
            }

            return out;
        }

    public:
        const size_t size() const noexcept override { return m_length; }

        void set(const char *identifier, const char *value) override {
            setIdentifier(identifier);
            m_type = field_traits<const char *>::type;
            m_length = strlen(value) + 1;
            std::strncpy(reinterpret_cast<char *>(m_storage), value, m_length);
        }

        void set(const char *identifier, const void *value, const size_t length) noexcept override {
            setIdentifier(identifier);
            m_type = field_traits<byte *>::type;
            m_length = length;
            std::memcpy(m_storage, value, m_length);
        }

        const size_t get(const byte **data) const noexcept override {
            assert(m_type == StorageType::data);
            *data = m_storage;
            return m_length;
        }

        const size_t get(const char **data) const noexcept override {
            assert(m_type == StorageType::string);
            *data = reinterpret_cast<const char *>(m_storage);
            return m_length;
        }

        template <typename T, typename = std::enable_if<field_traits<T>::value && std::is_pointer<T>::value>> const T get() const noexcept {
            return DataField::get<T>();
        };

        const size_t encode(MessageBuffer &buffer) const noexcept override {
            byte *b = buffer.allocate(MsgField::size());

            b = writeScalar(b, static_cast<MsgField::type>(m_type));

            const size_t identifier_length = strlen(m_identifier);
            b = writeScalar(b, static_cast<MsgField::identifier_length >(identifier_length));

            b = writeScalar(b, static_cast<MsgField::data_length>(m_length));

            buffer.append(reinterpret_cast<const byte *>(m_identifier), identifier_length);
            buffer.append(m_storage, m_length);

            return MsgField::size() + identifier_length + m_length;
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
                        m_length = size;
                        std::memcpy(m_storage, buffer.readBytes(), size);
                        buffer.advanceRead(size);
                        return true;
                    }
                }
            }

            return false;
        }
    };
}

#endif //TFDCF_SMALLDATAFIELD_H
