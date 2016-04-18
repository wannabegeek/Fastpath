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

#ifndef FASTPATH_MESSAGEFIELD_H
#define FASTPATH_MESSAGEFIELD_H

#include "fastpath/messages/Field.h"
#include "fastpath/messages/BaseMessage.h"

namespace fp {
    class MessageField final : public Field {
    private:
        BaseMessage m_msg;

    protected:
        virtual const bool isEqual(const Field &other) const noexcept override {
            if (typeid(other) == typeid(MessageField)) {
                const MessageField &f = static_cast<const MessageField &>(other);
                return m_msg == f.m_msg;
            }
            return false;
        }

        virtual std::ostream& output(std::ostream& out) const override;

    public:
//        MessageField(const char *identifier, const BaseMessage message) noexcept;
        MessageField(const char *identifier, BaseMessage &&message) noexcept;
        MessageField(const MessageBuffer::ByteStorageType &buffer) throw(fp::exception);

        const BaseMessage *get() const;

        const size_t encode(MessageBuffer &buffer) const noexcept override;
    };
}
#endif //FASTPATH_MESSAGEFIELD_H
