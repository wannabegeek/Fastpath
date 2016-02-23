//
// Created by Tom Fewster on 17/02/2016.
//

#include <string.h>
#include "MutableByteStorage.h"
#include <algorithm>

namespace DCF {
    MutableByteStorage::MutableByteStorage() {
        // call super classes protected constructor
    }

    MutableByteStorage::MutableByteStorage(byte *bytes, size_t length) : ByteStorage(bytes, length, false) {
    }

    void MutableByteStorage::setData(const byte *data, const size_t length) {
        if (length > m_storage.second) {
            delete[] m_storage.first;
            allocateStorage(std::max(length, m_storage.second * 2));
        }
        memcpy(m_storage.first, data, length);
        m_storedLength = length;
    }

    void MutableByteStorage::increaseLengthBy(const size_t length) noexcept {
        if (m_storedLength + length > m_storage.second) {
            const byte *old_data = m_storage.first;
            const size_t old_length = m_storage.second;
            allocateStorage(m_storage.second + length);
            memcpy(m_storage.first, old_data, old_length);
            delete[] old_data;
        }
        m_storedLength += length;
    }


    void MutableByteStorage::append(const byte *buffer, const size_t length) noexcept {
        this->increaseLengthBy(length);
        memcpy(m_storage.first, buffer, length);
    }

    void MutableByteStorage::append(const ByteStorage &src, const size_t length) noexcept {
        this->increaseLengthBy(length);
        const byte *data = nullptr;
        const size_t availableData = src.bytes(&data);
        memcpy(m_storage.first, data, std::max(length, availableData));
    }

}