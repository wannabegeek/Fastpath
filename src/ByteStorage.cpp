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

    void ByteStorage::mark() const noexcept {
        m_mark_ptr = m_read_ptr;
    }

    void ByteStorage::resetRead() const noexcept {
        m_read_ptr = m_mark_ptr;
    }

    void ByteStorage::advanceRead(const size_t distance) const noexcept {
        std::advance(m_read_ptr, distance);
    }

    const size_t ByteStorage::remainingReadLength() const noexcept {
        size_t v =  m_storedLength - this->bytesRead();
        return v;
    }

    const size_t ByteStorage::bytesRead() const noexcept {
        return m_read_ptr - m_storage.first;
    }

    const byte *ByteStorage::readBytes() const noexcept {
        return m_read_ptr;
    }

    const byte *ByteStorage::operator*() const noexcept {
        return this->readBytes();
    }

    const ByteStorage ByteStorage::segment(const size_t length) const noexcept {
        assert(length <= remainingReadLength());
        auto ptr = m_read_ptr;
        advanceRead(length);
        return ByteStorage(ptr, length, true);
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