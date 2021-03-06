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

#include "fastpath/utils/logger.h"
#include "fastpath/messages/Message.h"
#include "fastpath/messages/FieldAllocator.h"

namespace fp {

    Message::Message() noexcept : m_hasAddressing(true), m_flags(-1) {
        m_subject[0] = '\0';
    }

    Message::Message(Message &&other) noexcept : BaseMessage(std::move(other)),
                                                             m_hasAddressing(other.m_hasAddressing),
                                                             m_flags(other.m_flags) {
        std::strcpy(m_subject, other.m_subject);
        other.m_subject[0] = '\0';
    }

    Message::~Message() noexcept {
    }

    const bool Message::operator==(const Message &other) const {
        return m_flags == other.m_flags
               && strncmp(m_subject, other.m_subject, max_subject_length) == 0
               && BaseMessage::operator==(other);
    }


    void Message::clear() noexcept {
        m_flags = -1;
        m_subject[0] = '\0';
        BaseMessage::clear();
    }

    std::ostream &Message::output(std::ostream &out) const {
        if (m_hasAddressing) {
            if (m_subject[0] == '\0') {
                out << "<no subject>=";
            } else {
                out << m_subject << "=";
            }
        }
        return BaseMessage::output(out);
    }

//#ifdef DEBUG
//    void Message::logMessageBufferDetails(const MessageBuffer::ByteStorageType &buffer) noexcept {
//        std::stringstream s;
//        s << "Buffer reports there are " << buffer.remainingReadLength() << " bytes remaining to read from a buffer of " << buffer.length() << " bytes" << std::endl;
//
//        size_t subject_length = 0;
//        size_t msg_length = 0;
//        uint8_t flags = 0;
//        const char *subject = nullptr;
//        buffer.mark();
//        auto status = Message::addressing_details(buffer, &subject, subject_length, flags, msg_length);
//        buffer.resetRead();
//        if (status == CompleteMessage) {
//            MessageBuffer::ByteStorageType msg_buffer(buffer.readBytes(), msg_length, false);
//            s << "We have a complete message, consisting of;" << std::endl;
//            s << "\t Msg Length: " << msg_length << " bytes" << std::endl;
//            s << "\tSubject: " << std::string(subject, subject_length) << std::endl;
//            s << "Buffer content: " << msg_buffer << std::endl;
//            msg_buffer.advanceRead(MsgAddressing::size());
//            msg_buffer.advanceRead(subject_length);
//
//            MsgHeader::header_start chk = readScalar<MsgHeader::header_start>(msg_buffer.readBytes());
//            msg_buffer.advanceRead(sizeof(MsgHeader::header_start));
//            if (chk != 2) {
//                s << "*** Missing body flag ***";
//            } else {
//                const MsgHeader::field_count field_count = readScalar<MsgHeader::field_count>(msg_buffer.readBytes());
//                msg_buffer.advanceRead(sizeof(MsgHeader::field_count));
//                s << "Field count: " << field_count << std::endl;
//                for (size_t i = 0; i < field_count; i++) {
//                    MsgField::type type = unknown;
//                    MsgField::identifier_length identifier_size = 0;
//                    MsgField::data_length data_size = 0;
//                    Field::peek_field_header(msg_buffer, type, identifier_size, data_size);
//                    msg_buffer.advanceRead(MsgField::size());
//                    msg_buffer.advanceRead(identifier_size);
//                    msg_buffer.advanceRead(data_size);
//                    s << "\tField is type:    " << StorageTypeDescription[static_cast<int>(type)] << std::endl;
//                    s << "\tIdentifier Length: " << static_cast<int>(identifier_size) << std::endl;
//                    s << "\tData Length        : " << data_size << std::endl;
//                }
//            }
//        } else if (status == IncompleteMessage) {
//            s << "We have an incomplete message, we require " << msg_length << " bytes but have " << buffer.remainingReadLength() << " bytes" << std::endl;
//        } else if (status == CorruptMessage) {
//            s << "**** Received corrupt message ****";
//        }
//
//        INFO_LOG(s.str());
//    }
//#endif
}
