//
// Created by Tom Fewster on 25/02/2016.
//

#include "MessageField.h"
#include "Message.h"

namespace DCF {
//    MessageField::MessageField(const char *identifier, const BaseMessage message) noexcept : m_msg(msg){
//        setIdentifier(identifier);
//    }

    MessageField::MessageField(const char *identifier, BaseMessage &&message) noexcept : m_msg(std::move(message)) {
        DEBUG_LOG("In constructor");
        setIdentifier(identifier);
    }

    MessageField::MessageField(const MessageBuffer::ByteStorageType &buffer) noexcept {
        if (buffer.remainingReadLength() >= MsgField::size()) {
            assert(static_cast<StorageType>(readScalar<MsgField::type>(buffer.readBytes())) == StorageType::message);
            buffer.advanceRead(sizeof(MsgField::type));

            const size_t identifier_length = readScalar<MsgField::identifier_length>(buffer.readBytes());
            buffer.advanceRead(sizeof(MsgField::identifier_length));

            const size_t size = readScalar<MsgField::data_length>(buffer.readBytes());
            buffer.advanceRead(sizeof(MsgField::data_length));

            if (buffer.remainingReadLength() >= identifier_length) {
                std::copy(buffer.readBytes(), &buffer.readBytes()[identifier_length], m_identifier);
                m_identifier[identifier_length] = '\0';
                buffer.advanceRead(identifier_length);

                if (buffer.remainingReadLength() >= size) {
                    m_msg.decode(buffer);
                }
            }
        }

        return; // false;
    }

//    void MessageField::set(const char *identifier, const BaseMessage msg) {
//        setIdentifier(identifier);
//        m_msg.reset(msg);
//    }
//
//    void MessageField::set(const char *identifier, BaseMessage &&msg) {
//        setIdentifier(identifier);
//        m_msg = std::forward<BaseMessage>(msg);
//    }

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

    const bool MessageField::decode(const MessageBuffer::ByteStorageType &buffer) noexcept {
        return 0;
    }

    std::ostream &MessageField::output(std::ostream &out) const {
        return out << m_identifier << ":message=" << m_msg;
    }
}

