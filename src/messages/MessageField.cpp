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
    }

    const BaseMessage *MessageField::get() const {
        return &m_msg;
    }

    const size_t MessageField::encode(MessageBuffer &buffer) const noexcept {
        MessageBuffer storage(1024);
        m_msg.encode(storage);

        const byte *bytes = nullptr;
        size_t data_length = storage.bytes(&bytes);

        return Field::encode(buffer, bytes, data_length);
    }

    std::ostream &MessageField::output(std::ostream &out) const {
        return out << m_identifier << ":message=" << m_msg;
    }
}

