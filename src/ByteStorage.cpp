//
// Created by Tom Fewster on 23/02/2016.
//

#include <cstring>
#include <algorithm>
#include "ByteStorage.h"
#include "Types.h"

namespace DCF {
    ByteStorage::ByteStorage() : m_no_copy(false) {
        allocateStorage(256);
    }

    ByteStorage::ByteStorage(byte *bytes, size_t length, bool no_copy) : m_no_copy(no_copy) {
        if (m_no_copy) {
            m_storage.first = bytes;
            m_storage.second = 0;
            m_storedLength = length;
        } else {
            allocateStorage(length);
            memcpy(m_storage.first, reinterpret_cast<const void *>(data), length);
            m_storedLength = length;
        }
    }

    ByteStorage::~ByteStorage() {
        if (!m_no_copy) {
            delete[] m_storage.first;
        }
    }

    void ByteStorage::allocateStorage(const size_t length) {
        // this will find the next x^2 number larger than the one provided
        m_storage.second = length;
        m_storage.second--;
        m_storage.second |= m_storage.second >> 1;
        m_storage.second |= m_storage.second >> 2;
        m_storage.second |= m_storage.second >> 4;
        m_storage.second |= m_storage.second >> 8;
        m_storage.second |= m_storage.second >> 16;
        m_storage.second++;

        m_storage.first = new byte[m_storage.second];

    }

    const size_t ByteStorage::bytes(const byte **data) const {
        *data = m_storage.first;
        return m_storedLength;
    }
}