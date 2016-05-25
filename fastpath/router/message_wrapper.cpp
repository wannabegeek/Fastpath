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

#include "message_wrapper.h"

namespace fp {

    message_wrapper::message_wrapper(const MessageBuffer::ByteStorageType &buffer) noexcept : m_byte_storage(buffer) {
    }

    message_wrapper::message_wrapper(const MessageBuffer::ByteStorageType &buffer, const SharedMemoryManager::shared_ptr_type &shared_ptr) noexcept
            : m_byte_storage(buffer), m_shared_ptr(shared_ptr) {
    }

    const SharedMemoryManager::shared_ptr_type &message_wrapper::getSharedPtrBuffer() const noexcept {
        if (m_shared_ptr) {
            return m_shared_ptr;
        } else {
            ERROR_LOG("We need to create a shared ptr buffer");
            return m_shared_ptr;
        }
    }

}