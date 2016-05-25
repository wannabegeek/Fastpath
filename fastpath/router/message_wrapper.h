//
// Created by Tom Fewster on 23/05/2016.
//

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
