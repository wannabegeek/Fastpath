//
// Created by Tom Fewster on 12/02/2016.
//

#ifndef TFDCF_ELEMENT_H
#define TFDCF_ELEMENT_H

#include <stddef.h>
#include <string>
#include <type_traits>
#include <boost/any.hpp>
#include "Types.h"
#include "Encoder.h"
#include "ElementTraits.h"

/**
 * Binary Format:
 * | Field Name Len | Value Len | StorageType | Payload  |
 * |    16 Bytes    | 32 Bytes  |   8 Bytes   | Variable |
 */

namespace DCF {

    class Element : Encoder {
    private:
        boost::any m_value;
        StorageType m_type;
        size_t m_size;

    public:
//        template <typename T, typename = std::enable_if_t<is_valid_type<T>::value>> Element(const T &value) : m_value(value), m_type(is_valid_type<T>::type), m_size(sizeof(value)) {
//        }
//
////        template <typename T, typename = std::enable_if_t<is_valid_type<T>::value && std::is_same<T, std::string>::value>> Element(const T &&value) : m_value(value), m_type(is_valid_type<T>::type), m_size(sizeof(value)) {
////        }
//
//        Element(const void *value, const size_t size) : m_value(value), m_type(is_valid_type<void *>::type), m_size(size) {
//        }

        template <typename T> void setValue(const T &value) {
            m_value = value;
            m_type = is_valid_type<T>::type;
        }

        void setValue(const char *value);

        void setValue(const void *data, const size_t size);

//        std::enable_if< std::is_same< X, T >::value
//        template <> Element(const std::string &&value) : m_value(value), m_type(is_valid_type<std::string>::type), m_size(value.size()) {
//        }

        ~Element() {}

        const StorageType type() const {
            return m_type;
        }

        template <typename T> const bool get(T &value) const {
            try {
                value = boost::any_cast<T>(m_value);
            } catch(const boost::bad_any_cast &) {
                return false;
            }

            return true;
        }

        void encode() override {

        }
    };

    template <> void Element::setValue(const std::string &value);
    template <> const bool Element::get(std::string &value) const;
}

#endif //TFDCF_ELEMENT_H
