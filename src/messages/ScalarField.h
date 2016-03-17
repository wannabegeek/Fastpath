//
// Created by Tom Fewster on 24/02/2016.
//

#ifndef TFDCF_SCALARFIELD_H
#define TFDCF_SCALARFIELD_H
#include <stddef.h>
#include <string>
#include <type_traits>
#include <iostream>
#include <cassert>
#include <algorithm>
#include "StorageTypes.h"
#include "FieldTraits.h"
#include "Field.h"

namespace DCF {

    class ScalarField : public Field {

    private:
        byte m_raw[sizeof(float64_t)] = {0};
        StorageType m_type;
        size_t m_size;

    protected:
        virtual const bool isEqual(const Field &other) const noexcept override {
            try {
                const ScalarField &f = dynamic_cast<const ScalarField &>(other);
                return m_type == f.m_type
                       && m_size == f.m_size
                       && std::equal(std::begin(m_raw), std::end(m_raw), std::begin(f.m_raw));
            } catch (const std::bad_cast &e) {
                return false;
            }
        }

        virtual std::ostream& output(std::ostream& out) const override {
            switch (m_type) {
                case StorageType::boolean:
                    out << m_identifier << ":boolean=" << std::boolalpha << get<bool>();
                    break;
                case StorageType::uint8:
                    out << m_identifier << ":uint8=" << get<uint8_t>();
                    break;
                case StorageType::uint16:
                    out << m_identifier << ":uint16=" << get<uint16_t>();
                    break;
                case StorageType::uint32:
                    out << m_identifier << ":uint32=" << get<uint32_t>();
                    break;
                case StorageType::uint64:
                    out << m_identifier << ":uint64=" << get<uint64_t>();
                    break;
                case StorageType::int8:
                    out << m_identifier << ":int8=" << get<int8_t>();
                    break;
                case StorageType::int16:
                    out << m_identifier << ":int16=" << get<int16_t>();
                    break;
                case StorageType::int32:
                    out << m_identifier << ":int32=" << get<int32_t>();
                    break;
                case StorageType::int64:
                    out << m_identifier << ":int64=" << get<int64_t>();
                    break;
                case StorageType::float32:
                    out << m_identifier << ":float32=" << get<float32_t>();
                    break;
                case StorageType::float64:
                    out << m_identifier << ":float64=" << get<float64_t>();
                    break;
                default:
                    // we can't handle any other message type
                    break;
            }

            return out;
        }

    public:

        const StorageType type() const noexcept override { return m_type; }

        const size_t size() const noexcept override { return m_size; }

        template<typename T, typename = std::enable_if<field_traits<T>::value && std::is_arithmetic<T>::value>>
        void set(const char *identifier, const T &value) {
            setIdentifier(identifier);
            m_type = field_traits<T>::type;
            m_size = field_traits<T>::size;
            writeScalar(m_raw, value);
        }

        template<typename T, typename = std::enable_if<field_traits<T>::value && std::is_arithmetic<T>::value>>
        const T get() const {
            assert(m_type == field_traits<T>::type);
            return readScalar<T>(m_raw);
        }

        const size_t encode(MessageBuffer &buffer) const noexcept override {
            byte *b = buffer.allocate(MsgField::size());

            b = writeScalar(b, static_cast<MsgField::type>(m_type));

            const size_t identifier_length = strlen(m_identifier);
            b = writeScalar(b, static_cast<MsgField::identifier_length >(identifier_length));
            b = writeScalar(b, static_cast<MsgField::data_length>(m_size));

            buffer.append(reinterpret_cast<const byte *>(m_identifier), identifier_length);
            buffer.append(reinterpret_cast<const byte *>(m_raw), m_size);
            return MsgField::size() + identifier_length + m_size;
        }

        const bool decode(const ByteStorage &buffer) noexcept override {
            if (buffer.remainingReadLength() >= MsgField::size()) {

                m_type = static_cast<StorageType>(readScalar<MsgField::type>(buffer.readBytes()));
                buffer.advanceRead(sizeof(MsgField::type));

                const size_t identifier_length = readScalar<MsgField::identifier_length>(buffer.readBytes());
                buffer.advanceRead(sizeof(MsgField::identifier_length));

                m_size = readScalar<MsgField::data_length>(buffer.readBytes());
                buffer.advanceRead(sizeof(MsgField::data_length));

                if (buffer.length() >= MsgField::size() + identifier_length) {
                    std::copy(buffer.readBytes(), &buffer.readBytes()[identifier_length], m_identifier);
                    m_identifier[identifier_length] = '\0';
                    buffer.advanceRead(identifier_length);
                    if (buffer.length() >= MsgField::size() + identifier_length + m_size) {
                        std::copy(buffer.readBytes(), &buffer.readBytes()[m_size], m_raw);
                        buffer.advanceRead(m_size);
                        return true;
                    }
                }
            }
            return false;
        }
    };
}

#endif //TFDCF_SCALARFIELD_H
