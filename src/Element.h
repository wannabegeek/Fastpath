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
#include "MessageBuffer.h"

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
        template <typename T> void setValue(const T &value) {
            m_value = value;
            m_type = is_valid_type<T>::type;
            m_size = sizeof(T);
        }

        void setValue(const char *value);

        void setValue(const void *data, const size_t size);

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

        const bool get(const char **value) const;

        void encode() override {

        }
    };

    template <> void Element::setValue(const std::string &value);

}

#endif //TFDCF_ELEMENT_H
