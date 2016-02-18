//
// Created by Tom Fewster on 17/02/2016.
//

#include <string.h>
#include "ByteStorage.h"
#include <algorithm>

namespace DCF {
    ByteStorage::ByteStorage() {
        resizeBuffer(256);
    }

    ByteStorage::~ByteStorage() {
        delete[] m_storage.first;
    }

    void ByteStorage::storeData(const byte *data, const size_t length) {
        if (length > m_storage.second) {
            resizeBuffer(std::max(length, m_storage.second * 2));
        }
        memcpy(m_storage.first, data, length);
        m_storedLength = length;
    }


    void ByteStorage::resizeBuffer(const size_t length) {
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

    const size_t ByteStorage::retreiveData(const byte **data) const {
        *data = m_storage.first;
        return m_storedLength;
    }
}