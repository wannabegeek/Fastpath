//
// Created by Tom Fewster on 23/05/2016.
//

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