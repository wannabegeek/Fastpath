//
// Created by Tom Fewster on 12/02/2016.
//

#ifndef TFDCF_ELEMENT_H
#define TFDCF_ELEMENT_H

#include <stddef.h>
#include <string>
#include <type_traits>
#include <iostream>
#include "StorageTypes.h"
#include "FieldTraits.h"
#include "MessageBuffer.h"
#include "MutableByteStorage.h"
#include "Serializable.h"

/**
 * Binary Format:
 * | Field Name Len | Value Len | StorageType | Payload  |
 * |    16 Bytes    | 32 Bytes  |   8 Bytes   | Variable |
 */

namespace DCF {

    class Message;

    class Field : public Serializable {
    protected:
        char m_identifier[256];
        virtual std::ostream& output(std::ostream& out) const = 0;
        virtual const bool isEqual(const Field &other) const = 0;
    public:
        const char *identifier() const { return m_identifier; }

        virtual const StorageType type() const noexcept = 0;
        virtual const size_t size() const noexcept = 0;

        virtual const size_t encode(MessageBuffer &buffer) const noexcept override = 0;
        virtual const bool decode(const MessageBuffer::ByteStorageType &buffer) noexcept override = 0;

        void setIdentifier(const char *identifier) {
            strcpy(m_identifier, identifier);
        }

        virtual const bool operator==(const Field &other) const {
            return strcmp(m_identifier, other.m_identifier) == 0
                    && this->isEqual(other);
        }

        friend std::ostream &operator<<(std::ostream &out, const Field &field) {
            return field.output(out);
        }

        static inline bool peek_field_header(const MessageBuffer::ByteStorageType &buffer, MsgField::type &type, MsgField::identifier_length &identifier_length, MsgField::data_length &data_size) noexcept {
            const byte *bytes = buffer.readBytes();
            type = static_cast<StorageType>(readScalar<MsgField::type>(bytes));
            std::advance(bytes, sizeof(MsgField::type));
            identifier_length = readScalar<MsgField::identifier_length >(bytes);
            std::advance(bytes, sizeof(MsgField::identifier_length));
            data_size = readScalar<MsgField::data_length>(bytes);

            return true;
        }

        inline const size_t encode(MessageBuffer &buffer, const byte *data, const size_t data_length) const noexcept {
            byte *b = buffer.allocate(MsgField::size());
            b = writeScalar(b, static_cast<MsgField::type>(this->type()));

            const size_t identifier_length = strlen(m_identifier);
            b = writeScalar(b, static_cast<MsgField::identifier_length>(identifier_length));
            writeScalar(b, static_cast<MsgField::data_length>(data_length));

            buffer.append(reinterpret_cast<const byte *>(m_identifier), identifier_length);
            buffer.append(data, data_length);
            return MsgField::size() + identifier_length + data_length;
        }
    };
}

#endif //TFDCF_ELEMENT_H
