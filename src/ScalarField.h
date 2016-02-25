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
#include "Types.h"
#include "FieldTraits.h"
#include "Field.h"

namespace DCF {
    template<typename T> T endianScalar(T t) {
#ifdef __BIG_ENDIAN__
        if (sizeof(T) == 1) {   // Compile-time if-then's.
            return t;
        } else if (sizeof(T) == 2) {
            auto r = __builtin_bswap16(*reinterpret_cast<uint16_t *>(&t));
            return *reinterpret_cast<T *>(&r);
        } else if (sizeof(T) == 4) {
            auto r = __builtin_bswap32(*reinterpret_cast<uint32_t *>(&t));
            return *reinterpret_cast<T *>(&r);
        } else if (sizeof(T) == 8) {
            auto r = __builtin_bswap64(*reinterpret_cast<uint64_t *>(&t));
            return *reinterpret_cast<T *>(&r);
        } else {
            assert(0);
        }
#else
        return t;
#endif
    }

    template<typename T> T readScalar(const byte *p) {
        return endianScalar(*reinterpret_cast<const T *>(p));
    }

    template<typename T> void writeScalar(byte *p, T t) {
        *reinterpret_cast<T *>(p) = endianScalar(t);
    }

    class ScalarField : public Field {

    private:
        byte m_raw[sizeof(float64_t)];
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

        void encode(MessageBuffer &buffer) noexcept override {
        }

        const size_t decode(const ByteStorage &buffer) noexcept override {
            return 0;
        }

        friend std::ostream &operator<<(std::ostream &out, const ScalarField &field) {
            switch (field.type()) {
                case StorageType::boolean:
                    out << field.m_identifier << ":boolean=" << std::boolalpha << field.get<bool>();
                    break;
                case StorageType::uint8:
                    out << field.m_identifier << ":uint8=" << field.get<uint8_t>();
                    break;
                case StorageType::uint16:
                    out << field.m_identifier << ":uint16=" << field.get<uint16_t>();
                    break;
                case StorageType::uint32:
                    out << field.m_identifier << ":uint32=" << field.get<uint32_t>();
                    break;
                case StorageType::uint64:
                    out << field.m_identifier << ":uint64=" << field.get<uint64_t>();
                    break;
                case StorageType::int8:
                    out << field.m_identifier << ":int8=" << field.get<int8_t>();
                    break;
                case StorageType::int16:
                    out << field.m_identifier << ":int16=" << field.get<int16_t>();
                    break;
                case StorageType::int32:
                    out << field.m_identifier << ":int32=" << field.get<int32_t>();
                    break;
                case StorageType::int64:
                    out << field.m_identifier << ":int64=" << field.get<int64_t>();
                    break;
                case StorageType::float32:
                    out << field.m_identifier << ":float32=" << field.get<float32_t>();
                    break;
                case StorageType::float64:
                    out << field.m_identifier << ":float64=" << field.get<float64_t>();
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
