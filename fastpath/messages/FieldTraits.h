/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef FASTPATH_ELEMENTTRAITS_H
#define FASTPATH_ELEMENTTRAITS_H

#include <cstdint>
#include <iosfwd>

#include "fastpath/messages/StorageTypes.h"
#include "fastpath/types.h"

namespace fp {

    class BaseMessage;
//    class ScalarField;
//    class DataField;
//    class MessageField;

    template<typename T>
    struct field_traits {
        static const bool value = false;
        static const storage_type type = storage_type::unknown;
        static const size_t size = 0;
        const constexpr char *description() { return "unknown"; }
    };

    template<>
    struct field_traits<bool> {
        static const bool value = true;
        static const storage_type type = storage_type::boolean;
        static const size_t size = sizeof(bool);
        const constexpr char *description() { return "boolean"; }
    };

    template<>
    struct field_traits<uint8_t> {
        static const bool value = true;
        static const storage_type type = storage_type::uint8;
        static const size_t size = sizeof(uint8_t);
        const constexpr char *description() { return "uint8"; }
    };

    template<>
    struct field_traits<uint16_t> {
        static const bool value = true;
        static const storage_type type = storage_type::uint16;
        static const size_t size = sizeof(uint16_t);
        const constexpr char *description() { return "uint16"; }
    };

    template<>
    struct field_traits<uint32_t> {
        static const bool value = true;
        static const storage_type type = storage_type::uint32;
        static const size_t size = sizeof(uint32_t);
        const constexpr char *description() { return "uint32"; }
    };

    template<>
    struct field_traits<uint64_t> {
        static const bool value = true;
        static const storage_type type = storage_type::uint64;
        static const size_t size = sizeof(uint64_t);
        const constexpr char *description() { return "uint64"; }
    };

    template<>
    struct field_traits<int8_t> {
        static const bool value = true;
        static const storage_type type = storage_type::int8;
        static const size_t size = sizeof(int8_t);
        const constexpr char *description() { return "int8"; }
    };

    template<>
    struct field_traits<int16_t> {
        static const bool value = true;
        static const storage_type type = storage_type::int16;
        static const size_t size = sizeof(int16_t);
        const constexpr char *description() { return "int16"; }
    };

    template<>
    struct field_traits<int32_t> {
        static const bool value = true;
        static const storage_type type = storage_type::int32;
        static const size_t size = sizeof(int32_t);
        const constexpr char *description() { return "int32"; }
    };

    template<>
    struct field_traits<int64_t> {
        static const bool value = true;
        static const storage_type type = storage_type::int64;
        static const size_t size = sizeof(int64_t);
        const constexpr char *description() { return "int64"; }
    };

//    template <> struct field_traits<float16_t> {
//        static const bool value = true;
//        static const StorageType type = StorageType::float16;
//    };

    template<>
    struct field_traits<float32_t> {
        static const bool value = true;
        static const storage_type type = storage_type::float32;
        static const size_t size = sizeof(float32_t);
        const constexpr char *description() { return "float32"; }
    };

    template<>
    struct field_traits<float64_t> {
        static const bool value = true;
        static const storage_type type = storage_type::float64;
        static const size_t size = sizeof(float64_t);
        const constexpr char *description() { return "float64"; }
    };

    template<>
    struct field_traits<const char *> {
        static const bool value = true;
        static const storage_type type = storage_type::string;
        const constexpr char *description() { return "string"; }
    };

    template<>
    struct field_traits<std::string> {
        static const bool value = true;
        static const storage_type type = storage_type::string;
        const constexpr char *description() { return "string"; }
    };

    template<>
    struct field_traits<BaseMessage> {
        static const bool value = true;
        static const storage_type type = storage_type::message;
        const constexpr char *description() { return "message"; }
    };

    template<>
    struct field_traits<byte *> {
        static const bool value = true;
        static const storage_type type = storage_type::data;
        const constexpr char *description() { return "data"; }
    };

//    template<StorageType N> struct storage_traits {
//    };
//
//    template<> struct storage_traits<StorageType::boolean> {
//        using storage_type = ScalarField;
//        using field = field_traits<bool>;
//    };
//
//    template<> struct storage_traits<StorageType::int8> {
//        using storage_type = ScalarField;
//        using field = field_traits<int8_t>;
//    };
//
//    template<> struct storage_traits<StorageType::int16> {
//        using storage_type = ScalarField;
//        using field = field_traits<int16_t>;
//    };
//
//    template<> struct storage_traits<StorageType::int32> {
//        using storage_type = ScalarField;
//        using field = field_traits<int32_t>;
//    };
//
//    template<> struct storage_traits<StorageType::int64> {
//        using storage_type = ScalarField;
//        using field = field_traits<int64_t>;
//    };
//
//    template<> struct storage_traits<StorageType::uint8> {
//        using storage_type = ScalarField;
//        using field = field_traits<uint8_t>;
//    };
//
//    template<> struct storage_traits<StorageType::uint16> {
//        using storage_type = ScalarField;
//        using field = field_traits<uint16_t>;
//    };
//
//    template<> struct storage_traits<StorageType::uint32> {
//        using storage_type = ScalarField;
//        using field = field_traits<uint32_t>;
//    };
//
//    template<> struct storage_traits<StorageType::uint64> {
//        using storage_type = ScalarField;
//        using field = field_traits<uint64_t>;
//    };
//
//    template<> struct storage_traits<StorageType::float32> {
//        using storage_type = ScalarField;
//        using field = field_traits<float32_t>;
//    };
//
//    template<> struct storage_traits<StorageType::float64> {
//        using storage_type = ScalarField;
//        using field = field_traits<float64_t>;
//    };
//
//    template<> struct storage_traits<StorageType::message> {
//        using storage_type = MessageField;
//        using field = field_traits<float64_t>;
//    };

//    template<StorageType N>
//    class FieldTraits {
//    private:
//        const Field &m_field;
//
//    public:
//        FieldTraits(const Field &field) : m_field(field) { }
//
//        friend std::ostream &operator<<(std::ostream &out, const FieldTraits &msg) {
//            return out << "unknown";
//        }
//    };
//
//
//    template<>
//    class FieldTraits<StorageType::string> {
//    private:
//        const Field &m_field;
//
//    public:
//        FieldTraits(const Field &field) : m_field(field) { }
//
//        friend std::ostream &operator<<(std::ostream &out, const FieldTraits &traits) {
//            const byte *data;
//            const size_t size = traits.m_field.m_storage.retreiveData(&data);
//            out << "[string]" << std::string(reinterpret_cast<const char *>(data), size - 1); // -1 for NULL
//        }
//    };
}


#endif //FASTPATH_ELEMENTTRAITS_H
