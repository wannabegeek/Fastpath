//
// Created by Tom Fewster on 17/02/2016.
//

#include <string.h>
#include "MutableByteStorage.h"
#include <algorithm>

namespace DCF {
    MutableByteStorage::MutableByteStorage(const size_t allocation) : ByteStorage(allocation) {
        // call super classes protected constructor
    }

    MutableByteStorage::MutableByteStorage(const byte *bytes, size_t length) : ByteStorage(bytes, length, false) {
    }

    MutableByteStorage::MutableByteStorage(MutableByteStorage &&orig) : ByteStorage(std::move(orig)) {
    }

    void MutableByteStorage::setData(const byte *data, const size_t length) {
        if (length > m_storage.second) {
            storage_traits::deallocate(m_allocator, m_storage.first, m_storage.second);
            allocateStorage(std::max(length, m_storage.second * 2));
        }
        memcpy(m_storage.first, data, length);
        m_storedLength = length;
    }

    void MutableByteStorage::increaseLengthBy(const size_t length) noexcept {
        if (m_storedLength + length > m_storage.second) {
            byte *old_data = m_storage.first;
            const size_t old_length = m_storage.second;
            allocateStorage(m_storage.second + length);
            memmove(m_storage.first, old_data, old_length);
            storage_traits::deallocate(m_allocator, old_data, old_length);
        }
        m_storedLength += length;
    }


    void MutableByteStorage::append(const byte *buffer, const size_t length) noexcept {
        const size_t current_length = m_storedLength;
        this->increaseLengthBy(length);
        memcpy(&m_storage.first[current_length], buffer, length);
    }

    void MutableByteStorage::append(const ByteStorage &src, const size_t length) noexcept {
        const size_t current_length = m_storedLength;
        const byte *data = nullptr;
        const size_t copy_length = std::min(src.bytes(&data), length);
        this->increaseLengthBy(copy_length);

        memcpy(&m_storage.first[current_length], data, copy_length);
    }

    const size_t MutableByteStorage::capacity() const {
        return m_storage.second;
    }

    byte *MutableByteStorage::mutableBytes() const noexcept {
        return m_storage.first;
    }

    void MutableByteStorage::truncate(const size_t length) {
        m_storedLength = length;
    }
}