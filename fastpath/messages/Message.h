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

#ifndef FASTPATH_MESSAGEADDRESSING_H
#define FASTPATH_MESSAGEADDRESSING_H

#include "fastpath/messages/BaseMessage.h"

namespace fp {
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

#endif //FASTPATH_MESSAGEADDRESSING_H
