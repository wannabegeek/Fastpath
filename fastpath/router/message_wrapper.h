/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 26/03/2016

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

#ifndef FASTPATH_MESSAGE_WRAPPER_H
#define FASTPATH_MESSAGE_WRAPPER_H

#include "fastpath/MessageBuffer.h"
#include "fastpath/SharedMemoryBuffer.h"

namespace fp {
    class message_wrapper {
        const MessageBuffer::ByteStorageType &m_byte_storage;
        SharedMemoryManager::shared_ptr_type m_shared_ptr;

    public:
//        message_wrapper() noexcept;
        message_wrapper(const MessageBuffer::ByteStorageType &buffer) noexcept;
        message_wrapper(const MessageBuffer::ByteStorageType &buffer, const SharedMemoryManager::shared_ptr_type &shared_ptr) noexcept;

        const MessageBuffer::ByteStorageType &getEncodedBuffer() const noexcept {
            return m_byte_storage;
        };

        const SharedMemoryManager::shared_ptr_type &getSharedPtrBuffer() const noexcept;
    };
}

#endif //FASTPATH_MESSAGE_WRAPPER_H
