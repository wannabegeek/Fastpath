//
// Created by Tom Fewster on 23/02/2016.
//

#include <cstring>
#include <algorithm>
#include <ostream>
#include <iomanip>
#include "ByteStorage.h"
#include "Types.h"

namespace DCF {
    ByteStorage::ByteStorage(const size_t allocation) : m_no_copy(false), m_storedLength(0) {
        allocateStorage(allocation);
    }

    ByteStorage::ByteStorage(const byte *bytes, size_t length, bool no_copy) : m_no_copy(no_copy), m_storedLength(0) {
        if (m_no_copy) {
            m_storage.first = const_cast<byte *>(bytes);
            m_storage.second = 0;
            m_storedLength = length;
        } else {
            allocateStorage(length);
            memcpy(m_storage.first, reinterpret_cast<const void *>(bytes), length);
            m_storedLength = length;
        }
    }

    ByteStorage::ByteStorage(ByteStorage &&orig) : m_storage(orig.m_storage), m_no_copy(orig.m_no_copy), m_storedLength(orig.m_storedLength) {
        orig.m_storage.first = nullptr;
        orig.m_storage.second = 0;
        orig.m_storedLength = 0;
        orig.m_no_copy = true;
    }

    ByteStorage::~ByteStorage() noexcept {
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
        *data = m_storedLength > 0 ? m_storage.first : nullptr;
        return m_storedLength;
    }

    std::ostream &operator<<(std::ostream &out, const ByteStorage &msg) {
        const byte *data = nullptr;
        const size_t length = msg.bytes(&data);
        out << "[length: " << length << " capacity: " << msg.m_storage.second << "]: " << std::endl;

        const byte *output = nullptr;
        const size_t default_block = 8;
        size_t inc = 0;

        for (size_t i = 0; i < length; i += default_block) {
            inc = std::min(default_block, length - i);

            output = &data[i];

            out << std::setfill('0') << std::setw(5) << i << "   ";

            for (size_t j = 0; j < default_block; j++) {
                if (j < inc) {
                    out << std::setfill('0') << std::setw(2) << std::hex << static_cast<const int>(output[j]) << " ";
                } else {
                    out << "   ";
                }
            }
            out << std::dec << "        ";

            for (size_t j = 0; j < default_block; j++) {
                if (j < inc) {
                    if (output[j] < 32 || output[j] > 127) {
                        out << '.' << " ";
                    } else {
                        out << static_cast<const char>(output[j]) << " ";
                    }
                } else {
                    out << "  ";
                }
            }

            out << std::endl;
        }
        return out << std::endl;
    }
}