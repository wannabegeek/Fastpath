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

    template<typename T>
    struct is_valid_type {
        static const bool value = false;
        static const StorageType type = StorageType::unknown;
        static const size_t size = 0;
    };

    template<>
    struct is_valid_type<bool> {
        static const bool value = true;
        static const StorageType type = StorageType::boolean;
        static const size_t size = sizeof(bool);
    };

    template<>
    struct is_valid_type<uint8_t> {
        static const bool value = true;
        static const StorageType type = StorageType::uint8;
        static const size_t size = sizeof(uint8_t);
    };

    template<>
    struct is_valid_type<uint16_t> {
        static const bool value = true;
        static const StorageType type = StorageType::uint16;
        static const size_t size = sizeof(uint16_t);
    };

    template<>
    struct is_valid_type<uint32_t> {
        static const bool value = true;
        static const StorageType type = StorageType::uint32;
        static const size_t size = sizeof(uint32_t);
    };

    template<>
    struct is_valid_type<uint64_t> {
        static const bool value = true;
        static const StorageType type = StorageType::uint64;
        static const size_t size = sizeof(uint64_t);
    };

    template<>
    struct is_valid_type<int8_t> {
        static const bool value = true;
        static const StorageType type = StorageType::int8;
        static const size_t size = sizeof(int8_t);
    };

    template<>
    struct is_valid_type<int16_t> {
        static const bool value = true;
        static const StorageType type = StorageType::int16;
        static const size_t size = sizeof(int16_t);
    };

    template<>
    struct is_valid_type<int32_t> {
        static const bool value = true;
        static const StorageType type = StorageType::int32;
        static const size_t size = sizeof(int32_t);
    };

    template<>
    struct is_valid_type<int64_t> {
        static const bool value = true;
        static const StorageType type = StorageType::int64;
        static const size_t size = sizeof(int64_t);
    };

//    template <> struct is_valid_type<float16_t> {
//        static const bool value = true;
//        static const StorageType type = StorageType::float16;
//    };

    template<>
    struct is_valid_type<float32_t> {
        static const bool value = true;
        static const StorageType type = StorageType::float32;
        static const size_t size = sizeof(float32_t);
    };

    template<>
    struct is_valid_type<float64_t> {
        static const bool value = true;
        static const StorageType type = StorageType::float64;
        static const size_t size = sizeof(float64_t);
    };

    template<>
    struct is_valid_type<const char *> {
        static const bool value = true;
        static const StorageType type = StorageType::string;
    };

    template<>
    struct is_valid_type<std::string> {
        static const bool value = true;
        static const StorageType type = StorageType::string;
    };

    template<>
    struct is_valid_type<Message> {
        static const bool value = true;
        static const StorageType type = StorageType::message;
    };

    template<>
    struct is_valid_type<void *> {
        static const bool value = true;
        static const StorageType type = StorageType::data;
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
