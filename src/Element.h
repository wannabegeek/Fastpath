//
// Created by Tom Fewster on 12/02/2016.
//

#ifndef TFDCF_ELEMENT_H
#define TFDCF_ELEMENT_H

#include <stddef.h>
#include <string>
#include <boost/any.hpp>
#include "Types.h"
#include "Encoder.h"

/**
 * Binary Format:
 * | Field Name Len | Value Len | StorageType | Payload  |
 * |    16 Bytes    | 32 Bytes  |   8 Bytes   | Variable |
 */

namespace DCF {
    class Element : Encoder {
    private:
        StorageType m_type;
        std::string m_field;
        size_t m_size;

        boost::any m_payload;

    public:
        template <typename T> Element(const std::string &field, const T value) : m_field(field) {
            m_type = StorageType::string;
        }

        template <typename T> Element(const std::string &&field, const T value) : m_field(field) {
            m_type = StorageType::string;
        }

        ~Element() {}

        const StorageType type() const {
            return m_type;
        }



        void encode() override {

        }
    };
}

#endif //TFDCF_ELEMENT_H
