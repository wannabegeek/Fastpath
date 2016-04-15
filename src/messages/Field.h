/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef TFDCF_ELEMENT_H
#define TFDCF_ELEMENT_H

#include <cstddef>
#include <string>
#include <type_traits>
#include <iostream>
#include <Exception.h>

#include "utils/optimize.h"
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
        StorageType m_type;
        std::size_t m_data_length;
        virtual std::ostream& output(std::ostream& out) const = 0;
        virtual const bool isEqual(const Field &other) const noexcept = 0;
    public:
        Field(const char *identifier, const StorageType type, const std::size_t data_length) noexcept : m_type(type), m_data_length(data_length) {
            strcpy(m_identifier, identifier);
        }

        Field(const MessageBuffer::ByteStorageType &buffer) throw(fp::exception) {
            if (tf::likely(buffer.remainingReadLength() >= MsgField::size())) {
                m_type = static_cast<StorageType>(readScalar<MsgField::type>(buffer.readBytes()));
                buffer.advanceRead(sizeof(MsgField::type));

                const size_t identifier_length = readScalar<MsgField::identifier_length>(buffer.readBytes());
                buffer.advanceRead(sizeof(MsgField::identifier_length));

                m_data_length = readScalar<MsgField::data_length>(buffer.readBytes());
                buffer.advanceRead(sizeof(MsgField::data_length));

                if (tf::likely(buffer.remainingReadLength() >= identifier_length)) {
                    std::copy(buffer.readBytes(), &buffer.readBytes()[identifier_length], m_identifier);
                    m_identifier[identifier_length] = '\0';
                    buffer.advanceRead(identifier_length);

                    if (tf::likely(buffer.remainingReadLength() >= m_data_length)) {
                        return;
                    } else {
                        ERROR_LOG("Not enough data available to read body [have: " << buffer.remainingReadLength() << " require: " << MsgField::size() + identifier_length + m_data_length);
                    }
                }
            } else {
                ERROR_LOG("Not enough data available to read header [have: " << buffer.remainingReadLength() << " require: " << MsgField::size());
            };

            throw fp::exception("Decode failed");
        }

        const char *identifier() const { return m_identifier; }

        const StorageType type() const noexcept {
            return m_type;
        }

        const size_t size() const noexcept {
            return m_data_length;
        };

        virtual const size_t encode(MessageBuffer &buffer) const noexcept override = 0;

        virtual const bool operator==(const Field &other) const {
            return strcmp(m_identifier, other.m_identifier) == 0
                   && m_type == other.m_type
                   && m_data_length == other.m_data_length
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

        virtual const bool decode(const MessageBuffer::ByteStorageType &buffer) noexcept override final {
            // no-op - we reconstruct this via the constructor
            return false;
        }
    };
}

#endif //TFDCF_ELEMENT_H
