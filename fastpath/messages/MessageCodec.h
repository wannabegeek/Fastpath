/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 26/03/2016

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

#ifndef FASTPATH_MESSAGECODEC_H
#define FASTPATH_MESSAGECODEC_H

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <cstring>

#include "fastpath/MessageBuffer.h"
#include "fastpath/Exception.h"
#include "fastpath/types.h"
#include "fastpath/messages/BaseMessage.h"
#include "fastpath/messages/Message.h"
#include "fastpath/messages/Serializable.h"
#include "fastpath/messages/Field.h"
#include "FieldAllocator.h"

namespace fp {
    /*template <typename T> */class MessageCodec {
        static constexpr const uint8_t addressing_flag = 1;
        static constexpr const uint8_t body_flag = 2;

        struct MsgAddressing {
            using addressing_start = uint8_t;
            using msg_length = uint64_t;
            using flags = uint8_t;
            using reserved = uint32_t;
            using subject_length = uint16_t;

            static constexpr const size_t msg_length_offset() {
                return sizeof(addressing_start)
                       + sizeof(msg_length);
            }

            static constexpr const size_t size() {
                return sizeof(addressing_start)
                       + sizeof(msg_length)
                       + sizeof(flags)
                       + sizeof(reserved)
                       + sizeof(subject_length);
            }
        };
//  char *subject;

        struct MsgHeader {
            using header_start = uint8_t;
            using field_count = uint32_t;    // len:4 off:11  Number of fields in main body

            static constexpr const size_t size() {
                return sizeof(header_start)
                       + sizeof(field_count);
            }
        };

        struct MsgField {
            using type = int8_t;
            using identifier_length = uint8_t;
            using data_length = uint32_t;

            static constexpr const size_t size() {
                return sizeof(type)
                       + sizeof(identifier_length)
                       + sizeof(data_length);
            }
        };

    public:

        typedef enum {
            CompleteMessage,
            IncompleteMessage,
            CorruptMessage
        } MessageDecodeStatus;

        template <typename T = ByteStorage<byte>> static const MessageDecodeStatus have_complete_message(const T &buffer, std::size_t &msg_length) noexcept {
            if (buffer.remainingReadLength() >= MsgAddressing::size()) {
                const byte *bytes = buffer.readBytes();
                MsgAddressing::addressing_start chk = readScalar<MsgAddressing::addressing_start>(buffer.readBytes());
                if (chk != addressing_flag) {
                    return CorruptMessage;
                }
                std::advance(bytes, sizeof(MsgAddressing::addressing_start));
                msg_length = readScalar<MsgAddressing::msg_length>(bytes) + MsgAddressing::msg_length_offset();
                if (buffer.remainingReadLength() >= msg_length) {
                    return CompleteMessage;
                }
            }

            return IncompleteMessage;
        }

        template <typename T = ByteStorage<byte>> static const MessageDecodeStatus addressing_details(const T &buffer, const char **subject, std::size_t &subject_length, uint8_t &flags, std::size_t &msg_length) noexcept {
            const MessageDecodeStatus status = have_complete_message(buffer, msg_length);

            if (status == CompleteMessage) {
                buffer.advanceRead(sizeof(MsgAddressing::addressing_start));
                buffer.advanceRead(sizeof(MsgAddressing::msg_length));
                flags = readScalar<MsgAddressing::flags>(buffer.readBytes());
                buffer.advanceRead(sizeof(MsgAddressing::flags));

                buffer.advanceRead(sizeof(MsgAddressing::reserved));

                subject_length = readScalar<MsgAddressing::subject_length>(buffer.readBytes());
                buffer.advanceRead(sizeof(MsgAddressing::subject_length));

                *subject = reinterpret_cast<const char *>(buffer.readBytes());
                buffer.advanceRead(subject_length);
            }

            return status;
        }


        template <typename T = MutableByteStorage<byte>> static const std::size_t encode(const BaseMessage *msg, T &buffer) noexcept {
            buffer.appendScalar(static_cast<MsgHeader::header_start>(body_flag));
            buffer.appendScalar(static_cast<MsgHeader::field_count>(msg->size()));

            std::size_t msgLength = MsgHeader::size();
            for (const Field *field : msg->m_payload) {
                msgLength += field->encode(buffer);
            }

            return msgLength;
        }

        template <typename T = MutableByteStorage<byte>> static const std::size_t encode(const Message *msg, T &buffer) noexcept {
            std::size_t addressing_size = MsgAddressing::size();

            buffer.appendScalar(static_cast<MsgAddressing::addressing_start>(addressing_flag));
            byte *length_ptr = buffer.allocate(sizeof(MsgAddressing::msg_length));
            buffer.appendScalar(static_cast<MsgAddressing::flags>(msg->flags()));
            buffer.appendScalar(static_cast<MsgAddressing::reserved>(0));
            const std::size_t subject_length = strlen(msg->subject()) + 1; // +1 for the terminating null, this make processing easier later
            buffer.appendScalar(static_cast<MsgAddressing::subject_length>(subject_length));

            buffer.append(reinterpret_cast<const byte *>(msg->subject()), subject_length);
            addressing_size += subject_length;

            const std::size_t total_len = addressing_size + encode(static_cast<const BaseMessage *>(msg), buffer);

            writeScalar(length_ptr, static_cast<MsgAddressing::msg_length>(total_len - MsgAddressing::msg_length_offset()));

            return total_len;
        }

        template <typename T = ByteStorage<byte>> static const bool decode(Message *msg, const T &buffer) throw (fp::exception) {
            size_t subject_length = 0;
            size_t msg_length = 0;
            const char *subject = nullptr;
            auto status = addressing_details(buffer, &subject, subject_length, msg->m_flags, msg_length);
            switch (status) {
                case CompleteMessage:
                    std::copy(subject, &subject[subject_length], msg->m_subject);
                    return decode(static_cast<BaseMessage *>(msg), buffer);
                case CorruptMessage:
                    throw fp::exception("Received corrupt message - incorrect addressing marker");
                case IncompleteMessage:
                    break;
            }

            return false;
        }

        template <typename T = ByteStorage<byte>> static const bool decode(BaseMessage *msg, const T &buffer) throw (fp::exception) {
            bool success = false;
            assert(buffer.length() > 0);

            if (buffer.remainingReadLength() > MsgHeader::size()) {
                MsgHeader::header_start chk = buffer.template readScalar<MsgHeader::header_start>();
                if (chk != body_flag) {
                    throw fp::exception("Received corrupt message - incorrect body marker");
                }

                const MsgHeader::field_count field_count = buffer.template readScalar<MsgHeader::field_count>();

                for (size_t i = 0; i < field_count; i++) {
                    MsgField::type type = unknown;
                    MsgField::identifier_length identifier_size = 0;
                    MsgField::data_length data_size = 0;
                    Field::peek_field_header(buffer, type, identifier_size, data_size);

                    Field *field = nullptr;
                    switch (static_cast<storage_type>(type)) {
                        case storage_type::string:
                        case storage_type::data:
                            field = createDataField(msg->m_field_allocator, data_size, buffer);
                            break;
                        case storage_type::date_time:
                            field = createDateTimeField(msg->m_field_allocator, buffer);
                            break;
                        case storage_type::message:
                            field = createMessageField(msg->m_field_allocator, buffer);
                            break;
                        default:
                            field = createScalarField(msg->m_field_allocator, buffer);
                            break;
                    }

                    msg->m_keys.insert(std::make_pair(field->identifier(), msg->m_payload.size()));
                    msg->m_payload.emplace_back(field);
                    success = true;
                }
            }
            return success;
        }
    };
}

#endif //FASTPATH_MESSAGECODEC_H
