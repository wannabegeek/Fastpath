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

#ifndef TFDCF_DATAFIELD_H
#define TFDCF_DATAFIELD_H

#include "fastpath/messages/Field.h"

namespace DCF {
    class DataField : public Field {

    protected:
        virtual const bool isEqual(const Field &other) const noexcept override = 0;
    public:
//        DataField(const char *identifier) noexcept : Field(identifier) {}
        DataField(const char *identifier, const StorageType &type, const std::size_t data_length) noexcept : Field(identifier, type, data_length) {}
        DataField(const MessageBuffer::ByteStorageType &buffer) throw(fp::exception) : Field(buffer) {}

        virtual ~DataField() {}

        virtual const size_t get(const byte **data) const noexcept = 0;
        virtual const size_t get(const char **data) const noexcept = 0;

        template <typename T, typename = std::enable_if<field_traits<T>::value && std::is_pointer<T>::value>> const T get() const noexcept {
            assert(m_type == field_traits<T>::type);
            typename std::remove_const<T>::type bytes = nullptr;
            this->get(&bytes);
            return bytes;
        }

        virtual const size_t encode(MessageBuffer &buffer) const noexcept override = 0;
    };
}

#endif //TFDCF_DATAFIELD_H
