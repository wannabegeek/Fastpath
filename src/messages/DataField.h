//
// Created by Tom Fewster on 24/02/2016.
//

#ifndef TFDCF_DATAFIELD_H
#define TFDCF_DATAFIELD_H

#include "Field.h"

namespace DCF {
    class DataField : public Field {

    protected:
        virtual const bool isEqual(const Field &other) const noexcept override = 0;
    public:
//        DataField(const char *identifier) noexcept : Field(identifier) {}
        DataField(const char *identifier, const StorageType &type, const std::size_t data_length) noexcept : Field(identifier, type, data_length) {}
        DataField(const MessageBuffer::ByteStorageType &buffer) throw(fp::exception) : Field(buffer) {}

        virtual ~DataField() {}

        virtual const size_t get(const byte **data) const noexcept = 0;
        virtual const size_t get(const char **data) const noexcept = 0;

        template <typename T, typename = std::enable_if<field_traits<T>::value && std::is_pointer<T>::value>> const T get() const noexcept {
            assert(m_type == field_traits<T>::type);
            typename std::remove_const<T>::type bytes = nullptr;
            this->get(&bytes);
            return bytes;
        }

        virtual const size_t encode(MessageBuffer &buffer) const noexcept override = 0;
    };
}

#endif //TFDCF_DATAFIELD_H
