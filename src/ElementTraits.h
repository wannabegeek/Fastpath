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

    template <typename T> struct is_valid_type {
        static const bool value = false;
    };

    template <> struct is_valid_type<bool> {
        static const bool value = true;
        static const StorageType type = StorageType::boolean;
    };

    template <> struct is_valid_type<uint8_t> {
        static const bool value = true;
        static const StorageType type = StorageType::uint8;
    };

    template <> struct is_valid_type<uint16_t> {
        static const bool value = true;
        static const StorageType type = StorageType::uint16;
    };

    template <> struct is_valid_type<uint32_t> {
        static const bool value = true;
        static const StorageType type = StorageType::uint32;
    };

    template <> struct is_valid_type<uint64_t> {
        static const bool value = true;
        static const StorageType type = StorageType::uint64;
    };

    template <> struct is_valid_type<int8_t> {
        static const bool value = true;
        static const StorageType type = StorageType::int8;
    };

    template <> struct is_valid_type<int16_t> {
        static const bool value = true;
        static const StorageType type = StorageType::int16;
    };

    template <> struct is_valid_type<int32_t> {
        static const bool value = true;
        static const StorageType type = StorageType::int32;
    };

    template <> struct is_valid_type<int64_t> {
        static const bool value = true;
        static const StorageType type = StorageType::int64;
    };

//    template <> struct is_valid_type<float16_t> {
//        static const bool value = true;
//        static const StorageType type = StorageType::float16;
//    };

    template <> struct is_valid_type<float32_t> {
        static const bool value = true;
        static const StorageType type = StorageType::float32;
    };

    template <> struct is_valid_type<float64_t> {
        static const bool value = true;
        static const StorageType type = StorageType::float64;
    };

    template <> struct is_valid_type<const char *> {
        static const bool value = true;
        static const StorageType type = StorageType::string;
    };

    template <> struct is_valid_type<std::string> {
        static const bool value = true;
        static const StorageType type = StorageType::string;
    };

    template <> struct is_valid_type<Message> {
        static const bool value = true;
        static const StorageType type = StorageType::message;
    };

    template <> struct is_valid_type<void *> {
        static const bool value = true;
        static const StorageType type = StorageType::data;
    };
}

#endif //TFDCF_ELEMENTTRAITS_H
