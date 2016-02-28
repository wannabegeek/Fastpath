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
#include "Types.h"
#include "FieldTraits.h"
#include "Field.h"

namespace DCF {

    class ScalarField : public Field {

    private:
        byte m_raw[sizeof(float64_t)] = {0};
        StorageType m_type;
        size_t m_size;

    public:

        const StorageType type() const noexcept override { return m_type; }
        const size_t size() const noexcept override { return m_size; }

        template <typename T, typename = std::enable_if<field_traits<T>::value && std::is_arithmetic<T>::value>> void set(const uint16_t identifier, const T &value) {
            m_identifier = identifier;
            m_type = field_traits<T>::type;
            m_size = field_traits<T>::size;
            writeScalar(m_raw, value);
        }

        template <typename T, typename = std::enable_if<field_traits<T>::value && std::is_arithmetic<T>::value>> const T get() const {
            assert(m_type == field_traits<T>::type);
            return readScalar<T>(m_raw);
        }

        const bool operator==(const ScalarField &other) const {
            return m_identifier == other.m_identifier
                   && m_type == other.m_type
                   && m_size == other.m_size
                   && std::equal(std::begin(m_raw), std::end(m_raw), std::begin(other.m_raw));
        }

        const size_t encode(MessageBuffer &buffer) noexcept override {
            byte *b = buffer.allocate(MsgField::size());

            b = writeScalar(b, static_cast<MsgField::type>(m_type));
            b = writeScalar(b, static_cast<MsgField::identifier>(m_identifier));
            b = writeScalar(b, static_cast<MsgField::data_length>(m_size));
            buffer.append(reinterpret_cast<const byte *>(m_raw), m_size);
            return MsgField::size() + m_size;
        }

        const bool decode(const ByteStorage &buffer, size_t &read_offset) noexcept override {
            if (buffer.length() >= MsgField::size()) {
                const byte *data = nullptr;
                buffer.bytes(&data);

                m_type = static_cast<StorageType>(readScalar<MsgField::type>(data));
                data += sizeof(MsgField::type);

                m_identifier = readScalar<MsgField::identifier>(data);
                data += sizeof(MsgField::identifier);

                m_size = readScalar<MsgField::data_length>(data);
                data += sizeof(MsgField::data_length);

                if (buffer.length() >= MsgField::size() + m_size) {
                    memcpy(m_raw, data, m_size);
                    read_offset = MsgField::size() + m_size;
                    return true;
                }
            }
            return false;
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
    };
}

#endif //TFDCF_SCALARFIELD_H
