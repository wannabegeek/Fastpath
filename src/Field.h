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

    class Message;

    typedef union {
        int8_t i8;
        uint8_t u8;
        int16_t i16;
        uint16_t u16;
        int32_t i32;
        uint32_t u32;
        int64_t i64;
        uint64_t u64;
        float32_t f32;
        float64_t f64;
        bool boolean;
        byte raw[sizeof(float64_t)];
    } DCMsgData;

    class Field : Encoder {
    private:
        DCMsgData m_data;
        StorageType m_type;
        size_t m_size;

        ByteStorage m_storage;

    public:

        const StorageType type() const { return m_type; }
        const size_t size() const { return m_size; }

        void setValue(const int8_t &value);
        void setValue(const uint8_t &value);
        void setValue(const int16_t &value);
        void setValue(const uint16_t &value);
        void setValue(const int32_t &value);
        void setValue(const uint32_t &value);
        void setValue(const int64_t &value);
        void setValue(const uint64_t &value);
        void setValue(const float32_t &value);
        void setValue(const float64_t &value);
        void setValue(const bool &value);

        void setValue(const Message &value);
        void setValue(const std::string &value);

        void setValue(const char *value);
        void setValue(const byte *data, const size_t size);

        const bool get(int8_t &value) const;
        const bool get(uint8_t &value) const;
        const bool get(int16_t &value) const;
        const bool get(uint16_t &value) const;
        const bool get(int32_t &value) const;
        const bool get(uint32_t &value) const;
        const bool get(int64_t &value) const;
        const bool get(uint64_t &value) const;
        const bool get(float32_t &value) const;
        const bool get(float64_t &value) const;
        const bool get(bool &value) const;

        const bool get(std::string &value) const;

        const bool get(const char **value) const;
        const bool get(const byte **value, size_t &size) const;

        void encode(MessageBuffer &buffer) noexcept override {
            byte *b = buffer.allocate(sizeof(Field));
            MsgField *field = reinterpret_cast<MsgField *>(b);
            field->identifier = 123;
            field->type = m_type;
            field->data_length = m_size;
            switch (field->type) {
                case StorageType::string:
                case StorageType::data: {
                    const byte *data;
                    const size_t size = m_storage.retreiveData(&data);
                    buffer.append(reinterpret_cast<const byte *>(data), size);
                    break;
                }
                case StorageType::message:
                    break;
                default:
                    buffer.append(reinterpret_cast<const byte *>(m_data.raw), m_size);
            }
        }
    };
}

#endif //TFDCF_ELEMENT_H
