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
    class Message : public BaseMessage {
        friend class MessageCodec;
    public:
        static constexpr size_t max_subject_length = std::numeric_limits<uint16_t>::max();
    private:
        bool m_hasAddressing;

        virtual std::ostream& output(std::ostream& out) const override;

    protected:
        uint8_t m_flags;
        char m_subject[max_subject_length];

    public:
        Message() noexcept;
        Message(Message &&msg) noexcept;
        virtual ~Message() noexcept;

        const bool operator==(const Message &other) const;

        void clear() noexcept override;

        inline const char *subject() const noexcept { return m_subject; }

        inline const uint8_t flags() const noexcept { return m_flags; }

#ifdef DEBUG
        static void logMessageBufferDetails(const MessageBuffer::ByteStorageType &buffer) noexcept;
#endif
    };
}

#endif //FASTPATH_MESSAGEADDRESSING_H
