//
// Created by Tom Fewster on 16/02/2016.
//

#ifndef TFDCF_ELEMENTTRAITS_H
#define TFDCF_ELEMENTTRAITS_H

#include <cstdint>
#include <iosfwd>

#include "Types.h"

namespace DCF {

    class Message;
    class ScalarField;
    class DataField;

    template<typename T>
    struct field_traits {
        static const bool value = false;
        static const StorageType type = StorageType::unknown;
        static const size_t size = 0;
        const constexpr char *description() { return "unknown"; }
    };

    template<>
    struct field_traits<bool> {
        static const bool value = true;
        static const StorageType type = StorageType::boolean;
        static const size_t size = sizeof(bool);
        const constexpr char *description() { return "boolean"; }
    };

    template<>
    struct field_traits<uint8_t> {
        static const bool value = true;
        static const StorageType type = StorageType::uint8;
        static const size_t size = sizeof(uint8_t);
        const constexpr char *description() { return "uint8"; }
    };

    template<>
    struct field_traits<uint16_t> {
        static const bool value = true;
        static const StorageType type = StorageType::uint16;
        static const size_t size = sizeof(uint16_t);
        const constexpr char *description() { return "uint16"; }
    };

    template<>
    struct field_traits<uint32_t> {
        static const bool value = true;
        static const StorageType type = StorageType::uint32;
        static const size_t size = sizeof(uint32_t);
        const constexpr char *description() { return "uint32"; }
    };

    template<>
    struct field_traits<uint64_t> {
        static const bool value = true;
        static const StorageType type = StorageType::uint64;
        static const size_t size = sizeof(uint64_t);
        const constexpr char *description() { return "uint64"; }
    };

    template<>
    struct field_traits<int8_t> {
        static const bool value = true;
        static const StorageType type = StorageType::int8;
        static const size_t size = sizeof(int8_t);
        const constexpr char *description() { return "int8"; }
    };

    template<>
    struct field_traits<int16_t> {
        static const bool value = true;
        static const StorageType type = StorageType::int16;
        static const size_t size = sizeof(int16_t);
        const constexpr char *description() { return "int16"; }
    };

    template<>
    struct field_traits<int32_t> {
        static const bool value = true;
        static const StorageType type = StorageType::int32;
        static const size_t size = sizeof(int32_t);
        const constexpr char *description() { return "int32"; }
    };

    template<>
    struct field_traits<int64_t> {
        static const bool value = true;
        static const StorageType type = StorageType::int64;
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
        static const StorageType type = StorageType::float32;
        static const size_t size = sizeof(float32_t);
        const constexpr char *description() { return "float32"; }
    };

    template<>
    struct field_traits<float64_t> {
        static const bool value = true;
        static const StorageType type = StorageType::float64;
        static const size_t size = sizeof(float64_t);
        const constexpr char *description() { return "float64"; }
    };

    template<>
    struct field_traits<const char *> {
        static const bool value = true;
        static const StorageType type = StorageType::string;
        const constexpr char *description() { return "string"; }
    };

    template<>
    struct field_traits<std::string> {
        static const bool value = true;
        static const StorageType type = StorageType::string;
        const constexpr char *description() { return "string"; }
    };

    template<>
    struct field_traits<Message> {
        static const bool value = true;
        static const StorageType type = StorageType::message;
        const constexpr char *description() { return "message"; }
    };

    template<>
    struct field_traits<byte *> {
        static const bool value = true;
        static const StorageType type = StorageType::data;
        const constexpr char *description() { return "data"; }
    };

    template<StorageType N> struct storage_traits {
    };

    template<> struct storage_traits<StorageType::boolean> {
        using storage_type = ScalarField;
        using field = field_traits<bool>;
    };

    template<> struct storage_traits<StorageType::int8> {
        using storage_type = ScalarField;
        using field = field_traits<int8_t>;
    };

    template<> struct storage_traits<StorageType::int16> {
        using storage_type = ScalarField;
        using field = field_traits<int16_t>;
    };

    template<> struct storage_traits<StorageType::int32> {
        using storage_type = ScalarField;
        using field = field_traits<int32_t>;
    };

    template<> struct storage_traits<StorageType::int64> {
        using storage_type = ScalarField;
        using field = field_traits<int64_t>;
    };

    template<> struct storage_traits<StorageType::uint8> {
        using storage_type = ScalarField;
        using field = field_traits<uint8_t>;
    };

    template<> struct storage_traits<StorageType::uint16> {
        using storage_type = ScalarField;
        using field = field_traits<uint16_t>;
    };

    template<> struct storage_traits<StorageType::uint32> {
        using storage_type = ScalarField;
        using field = field_traits<uint32_t>;
    };

    template<> struct storage_traits<StorageType::uint64> {
        using storage_type = ScalarField;
        using field = field_traits<uint64_t>;
    };

    template<> struct storage_traits<StorageType::float32> {
        using storage_type = ScalarField;
        using field = field_traits<float32_t>;
    };

    template<> struct storage_traits<StorageType::float64> {
        using storage_type = ScalarField;
        using field = field_traits<float64_t>;
    };

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


#endif //TFDCF_ELEMENTTRAITS_H
