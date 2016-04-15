//
// Created by Tom Fewster on 15/03/2016.
//

#ifndef TFDCF_MESSAGEADDRESSING_H
#define TFDCF_MESSAGEADDRESSING_H

#include "BaseMessage.h"

namespace DCF {
    typedef enum {
        CompleteMessage,
        IncompleteMessage,
        CorruptMessage
    } MessageDecodeStatus;

    class Message final : public BaseMessage {
    public:
        static constexpr size_t max_subject_length = std::numeric_limits<uint16_t>::max();
    private:
        static constexpr const uint8_t addressing_flag = 1;

        uint8_t m_flags;
        bool m_hasAddressing;
        char m_subject[max_subject_length];

        virtual std::ostream& output(std::ostream& out) const override;
        const void encodeMsgLength(MessageBuffer &buffer, const MsgAddressing::msg_length length) const noexcept;

    public:
        Message();
        Message(Message &&msg) noexcept;
        virtual ~Message();

        const bool operator==(const Message &other) const;

        void clear() override;

        const char *subject() const { return m_subject; }
        const bool setSubject(const char *subject);

        const uint8_t flags() const noexcept { return m_flags; }

        const size_t encode(MessageBuffer &buffer) const noexcept override;
        const bool decode(const MessageBuffer::ByteStorageType &buffer) throw (fp::exception) override;

        static const MessageDecodeStatus have_complete_message(const MessageBuffer::ByteStorageType &buffer, size_t &msg_length) noexcept;
        static const MessageDecodeStatus addressing_details(const MessageBuffer::ByteStorageType &buffer, const char **subject, size_t &subject_length, uint8_t &flags, size_t &msg_length) noexcept;
#ifdef DEBUG
        static void logMessageBufferDetails(const MessageBuffer::ByteStorageType &buffer) noexcept;
#endif
    };
}

#endif //TFDCF_MESSAGEADDRESSING_H
