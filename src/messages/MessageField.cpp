//
// Created by Tom Fewster on 25/02/2016.
//

#include "MessageField.h"
#include "Message.h"

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

