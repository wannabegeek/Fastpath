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

#include "fastpath/messages/MessageField.h"
#include "fastpath/messages/Message.h"

namespace DCF {

    MessageField::MessageField(const char *identifier, BaseMessage &&message) noexcept : Field(identifier, StorageType::message, 0),  m_msg(std::move(message)) {
    }

    MessageField::MessageField(const MessageBuffer::ByteStorageType &buffer) throw(fp::exception) : Field(buffer) {
        m_msg.decode(buffer);
        m_data_length = 0;
    }

    const BaseMessage *MessageField::get() const {
        return &m_msg;
    }

    const size_t MessageField::encode(MessageBuffer &buffer) const noexcept {
//        MessageBuffer storage(1024);
//        m_msg.encode(storage);
//
//        const byte *bytes = nullptr;
//        size_t data_length = storage.bytes(&bytes);
//        std::size_t field_length = Field::encode(buffer, bytes, data_length)

        byte *b = buffer.allocate(MsgField::size());
        b = writeScalar(b, static_cast<MsgField::type>(this->type()));

        const size_t identifier_length = strlen(m_identifier);
        byte *data_len_ptr = writeScalar(b, static_cast<MsgField::identifier_length>(identifier_length));

        buffer.append(reinterpret_cast<const byte *>(m_identifier), identifier_length);
        std::size_t data_length = m_msg.encode(buffer);
        writeScalar(data_len_ptr, static_cast<MsgField::data_length>(data_length));

        return MsgField::size() + identifier_length + data_length;
    }

    std::ostream &MessageField::output(std::ostream &out) const {
        return out << m_identifier << ":" << StorageTypeDescription[m_type] << "=" << m_msg;
    }
}

