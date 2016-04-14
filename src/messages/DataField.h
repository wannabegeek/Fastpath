//
// Created by Tom Fewster on 24/02/2016.
//

#ifndef TFDCF_DATAFIELD_H
#define TFDCF_DATAFIELD_H

#include "Field.h"

namespace DCF {
    class DataField : public Field {

    protected:
        StorageType m_type;

        virtual const bool isEqual(const Field &other) const noexcept override {
            return m_type == other.type();
        }

    public:
        DataField() noexcept {}
        DataField(const StorageType &type) noexcept : m_type(type) {}

        virtual ~DataField() {}

        const StorageType type() const noexcept override { return m_type; }
        virtual const size_t size() const noexcept override = 0;

        virtual const size_t get(const byte **data) const noexcept = 0;
        virtual const size_t get(const char **data) const noexcept = 0;

        template <typename T, typename = std::enable_if<field_traits<T>::value && std::is_pointer<T>::value>> const T get() const noexcept {
            assert(m_type == field_traits<T>::type);
            typename std::remove_const<T>::type bytes = nullptr;
            this->get(&bytes);
            return bytes;
        }

        virtual const size_t encode(MessageBuffer &buffer) const noexcept override = 0;
        virtual const bool decode(const MessageBuffer::ByteStorageType &buffer) noexcept override = 0;
    };
}

#endif //TFDCF_DATAFIELD_H
