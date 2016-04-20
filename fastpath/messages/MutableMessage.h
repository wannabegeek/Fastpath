//
// Created by Tom Fewster on 20/04/2016.
//

#ifndef FASTPATH_MUTABLEMESSAGE_H
#define FASTPATH_MUTABLEMESSAGE_H

#include "fastpath/messages/Message.h"

namespace fp {
    class MutableMessage final : public Message {
    public:

        MutableMessage();
        MutableMessage(MutableMessage &&msg) noexcept;
        virtual ~MutableMessage();

        const bool operator==(const Message &other) const;

        /**
         * Adds a scalar field of a type `<T>` to the message.
         *
         * @param field The field identifier name.
         * @param value Sets the field value as this scalar value.
         * @return `true` if the field was successfully added, `false` otherwise
         */
        bool addScalarField(const char *field, const bool &value);
        bool addScalarField(const char *field, const int8_t &value);
        bool addScalarField(const char *field, const int16_t &value);
        bool addScalarField(const char *field, const int32_t &value);
        bool addScalarField(const char *field, const int64_t &value);
        bool addScalarField(const char *field, const uint8_t &value);
        bool addScalarField(const char *field, const uint16_t &value);
        bool addScalarField(const char *field, const uint32_t &value);
        bool addScalarField(const char *field, const uint64_t &value);
        bool addScalarField(const char *field, const float32_t &value);
        bool addScalarField(const char *field, const float64_t &value);

        /**
         * Adds a data field of a string type to the message.
         *
         * @param field The field identifier name.
         * @param value Sets the field value as this string.
         * @return `true` if the field was successfully added, `false` otherwise
         */
        bool addDataField(const char *field, const char *value);

        /**
         * Adds a data field containing opaque bytes type to the message.
         *
         * @param field The field identifier name.
         * @param value Sets the field value as byte array.
         * @param size The length of the data array to be added.
         * @return `true` if the field was successfully added, `false` otherwise
         */
        bool addDataField(const char *field, const byte *value, const size_t size);

        /**
         * Adds a field containing a sub-message to the message.
         * It is the responsibility of the application to make sure the
         * sub-message is valid until the message has been send successfully.
         *
         * @param field The field identifier name.
         * @param msg Sets the field value to this message.
         * @return `true` if the field was successfully added, `false` otherwise
         */
        bool addMessageField(const char *field, BaseMessage &&msg);

        /**
         * Adds a date-time field to the message.
         *
         * @param field The field identifier name.
         * @param time Sets the field value to this time.
         * @return `true` if the field was successfully added, `false` otherwise
         */
        bool addDateTimeField(const char *field, const std::chrono::time_point<std::chrono::system_clock> &time);

        /**
         * Adds a date-time field to the message.
         *
         * @param field The field identifier name.
         * @param time Sets the field value to this time.
         * @return `true` if the field was successfully added, `false` otherwise
         */
        bool addDateTimeField(const char *field, const std::chrono::microseconds &time);

        /**
         * Adds a date-time field to the message.
         *
         * @param field The field identifier name.
         * @param seconds Sets the field value with seconds.
         * @param microseconds Sets the field value with microseconds.
         * @return `true` if the field was successfully added, `false` otherwise
         */
        bool addDateTimeField(const char *field, const uint64_t seconds, const uint64_t microseconds);

        /**
         * Remove a field from the message.
         *
         * @param field The field identifier name.
         * @return `true` if the field was successfully removed, `false` otherwise
         */
        bool removeField(const char *field);
    };
}

#endif //FASTPATH_MUTABLEMESSAGE_H
