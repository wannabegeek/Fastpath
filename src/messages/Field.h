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
        virtual const bool decode(const ByteStorage &buffer) noexcept override = 0;

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


//        void encode(MessageBuffer &buffer) noexcept override {
//            byte *b = buffer.allocate(sizeof(Field));
//            MsgField *field = reinterpret_cast<MsgField *>(b);
//            field->identifier = 123;
//            field->type = m_type;
//            field->data_length = m_size;
//            switch (field->type) {
//                case StorageType::string:
//                case StorageType::data: {
//                    const byte *data;
//                    const size_t size = m_storage.bytes(&data);
//                    buffer.append(reinterpret_cast<const byte *>(data), size);
//                    break;
//                }
//                case StorageType::message:
//                    break;
//                default:
//                    buffer.append(reinterpret_cast<const byte *>(m_data.raw), m_size);
//            }
//        }
//
//        // from Decoder
//        const size_t decode(const ByteStorage &buffer) noexcept override {
//            const byte *bytes = nullptr;
//            const size_t length = buffer.bytes(&bytes);
//            const MsgField *field = reinterpret_cast<const MsgField *>(bytes);
//            m_type = static_cast<StorageType>(field->type);
//            m_size = field->data_length;
//
//            std::cout << "Decoding " << m_size << std::endl;
//
//            size_t offset = sizeof(MsgField);
//            switch (m_type) {
//                case StorageType::string:
//                case StorageType::data: {
//                    m_storage.setData(bytes, m_size);
//                    break;
//                }
//                case StorageType::message:
//                    break;
//                default:
//                    memcpy(m_data.raw, bytes, m_size);
//            }
//            return offset + m_size;
//        }

    };
}

#endif //TFDCF_ELEMENT_H
