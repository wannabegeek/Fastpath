//
// Created by Tom Fewster on 15/03/2016.
//

#include "Message.h"

namespace DCF {

    Message::Message() : m_flags(-1), m_hasAddressing(true) {
        m_subject = new char[max_subject_length];
        m_subject[0] = '\0';
    }

    Message::Message(Message &&other) noexcept : BaseMessage(std::move(*this)),
                                                             m_flags(other.m_flags),
                                                             m_hasAddressing(other.m_hasAddressing) {
        std::swap(m_subject, other.m_subject);
        other.m_subject[0] = '\0';
    }

    Message::~Message() {
        delete [] m_subject;
    }

    const bool Message::operator==(const Message &other) const {
        return m_flags == other.m_flags
               && strncmp(m_subject, other.m_subject, max_subject_length) == 0
               && BaseMessage::operator==(other);
    }


    const bool Message::setSubject(const char *subject) {
        if (strlen(subject) < max_subject_length) {
            strcpy(&m_subject[0], subject);
            return true;
        }

        return false;
    }

    void Message::clear() {
        m_flags = -1;
        m_subject[0] = '\0';
        BaseMessage::clear();
    }

    const void Message::encodeMsgLength(MessageBuffer &buffer, const MsgAddressing::msg_length length) const noexcept {
        if (m_hasAddressing) {
            const byte *b = nullptr;
            buffer.bytes(&b);
            std::advance(b, sizeof(MsgAddressing::addressing_start));
            writeScalar(const_cast<byte *>(b), static_cast<MsgAddressing::msg_length>(length - MsgAddressing::msg_length_offset()));
        }
    }

    const size_t Message::encode(MessageBuffer &buffer) const noexcept {
        byte *b = buffer.allocate(MsgAddressing::size());
        size_t addressing_size = MsgAddressing::size();

        b = writeScalar(b, static_cast<MsgAddressing::addressing_start>(addressing_flag));
        b = writeScalar(b, static_cast<MsgAddressing::msg_length>(0));
        b = writeScalar(b, static_cast<MsgAddressing::flags>(this->flags()));
        b = writeScalar(b, static_cast<MsgAddressing::reserved>(0));
        const size_t subject_length = strlen(this->subject()) + 1; // +1 for the terminating null, this make processing easier later
        b = writeScalar(b, static_cast<MsgAddressing::subject_length>(subject_length));

        buffer.append(reinterpret_cast<const byte *>(this->subject()), subject_length);
        addressing_size += subject_length;

        const size_t total_len = addressing_size + BaseMessage::encode(buffer);
        encodeMsgLength(buffer, total_len);

        return total_len;
    }

    const bool Message::addressing_details(const ByteStorage &buffer, const char **subject, size_t &subject_length, uint8_t &flags, size_t &length, bool advance_reader) {
        buffer.mark();
        bool result = false;

        if (buffer.remainingReadLength() >= sizeof(MsgAddressing::addressing_start)) {
            MsgAddressing::addressing_start chk = readScalar<MsgAddressing::addressing_start>(buffer.readBytes());
            buffer.advanceRead(sizeof(MsgAddressing::addressing_start));
            if (chk != addressing_flag) {
                throw fp::exception("Received corrupt message - incorrect addressing marker");
            }
        }

        if (buffer.remainingReadLength() >= MsgAddressing::size()) {
            MsgAddressing::msg_length msg_length = readScalar<MsgAddressing::msg_length>(buffer.readBytes());
            if (buffer.remainingReadLength() >= msg_length) {
                buffer.advanceRead(sizeof(MsgAddressing::msg_length));
                flags = readScalar<MsgAddressing::flags>(buffer.readBytes());
                buffer.advanceRead(sizeof(MsgAddressing::flags));

                buffer.advanceRead(sizeof(MsgAddressing::reserved));

                subject_length = readScalar<MsgAddressing::subject_length>(buffer.readBytes());
                buffer.advanceRead(sizeof(MsgAddressing::subject_length));

                // TODO: check we have enough length for the subject
                *subject = reinterpret_cast<const char *>(buffer.readBytes());
                buffer.advanceRead(subject_length);
                length = msg_length + MsgAddressing::msg_length_offset();
                result = true;
            }
        }

        if (!advance_reader || result == false) {
            buffer.resetRead();
        }
        return result;
    }

    const bool Message::decode(const ByteStorage &buffer) {
        this->clear();
        size_t subject_length = 0;
        size_t msg_length = 0;
        const char *subject = nullptr;
        if (Message::addressing_details(buffer, &subject, subject_length, m_flags, msg_length)) {
            std::copy(subject, &subject[subject_length], m_subject);
            return BaseMessage::decode(buffer);
        }
        return false;
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
}