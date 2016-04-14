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

    class ScalarField final : public Field {

    private:
        byte m_raw[sizeof(float64_t)] = {0};

    protected:
        virtual const bool isEqual(const Field &other) const noexcept override {
            if (typeid(other) == typeid(ScalarField)) {
                const ScalarField &f = static_cast<const ScalarField &>(other);
                return std::equal(std::begin(m_raw), std::end(m_raw), std::begin(f.m_raw));
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

    public:
        template<typename T, typename = std::enable_if<field_traits<T>::value && std::is_arithmetic<T>::value>> ScalarField(const char *identifier, const T &value) noexcept : Field(identifier, field_traits<T>::type, field_traits<T>::size) {
            writeScalar(m_raw, value);
        }

        ScalarField(const MessageBuffer::ByteStorageType &buffer) throw(fp::exception) : Field(buffer) {
            std::copy(buffer.readBytes(), &buffer.readBytes()[m_data_length], m_raw);
            buffer.advanceRead(m_data_length);
        }

        template<typename T, typename = std::enable_if<field_traits<T>::value && std::is_arithmetic<T>::value>>
        const T get() const noexcept {
            assert(m_type == field_traits<T>::type);
            return readScalar<T>(m_raw);
        }

        const size_t encode(MessageBuffer &buffer) const noexcept override {
            return Field::encode(buffer, reinterpret_cast<const byte *>(m_raw), m_data_length);
        }
    };
}

#endif //TFDCF_SCALARFIELD_H
