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
#include "FieldTraits.h"
#include "MessageBuffer.h"
#include "ByteStorage.h"

/**
 * Binary Format:
 * | Field Name Len | Value Len | StorageType | Payload  |
 * |    16 Bytes    | 32 Bytes  |   8 Bytes   | Variable |
 */

namespace DCF {

    class Field : Encoder {
    private:
        boost::any m_value;
        StorageType m_type;
        size_t m_size;

        ByteStorage m_storage;

    public:

        template <typename T> void setValue(const T &value) {
            m_value = value;
            m_type = is_valid_type<T>::type;
            m_size = sizeof(T);
        }
        void setValue(const char *value);
        void setValue(const byte *data, const size_t size);

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
        const bool get(const byte **value, size_t &size) const;

        void encode() override {

        }
    };

    template <> void Field::setValue(const std::string &value);
    template <> const bool Field::get(std::string &value) const;

}

#endif //TFDCF_ELEMENT_H
