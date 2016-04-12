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
        const StorageType type() const noexcept override { return m_type; }
        virtual const size_t size() const noexcept override = 0;

        virtual void set(const char *identifier, const char *value) = 0;
        virtual const size_t get(const byte **data) const noexcept = 0;
        virtual const size_t get(const char **data) const noexcept = 0;

        virtual const size_t encode(MessageBuffer &buffer) const noexcept override = 0;
        virtual const bool decode(const MessageBuffer::ByteStorageType &buffer) noexcept override = 0;
    };
}

#endif //TFDCF_DATAFIELD_H
