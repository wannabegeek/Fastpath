//
// Created by Tom Fewster on 23/02/2016.
//

#include <cstring>
#include <algorithm>
#include <ostream>
#include <iomanip>
#include <cassert>
#include "ByteStorage.h"
#include "types.h"

namespace DCF {
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

        m_storage.first = storage_traits::allocate(m_allocator, m_storage.second);
    }


    std::ostream &operator<<(std::ostream &out, const ByteStorage &msg)
}