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
    ByteStorage::ByteStorage(const size_t allocation) : m_storedLength(0), m_no_copy(false), m_read_ptr(m_storage.first) {
        allocateStorage(allocation);
    }

    ByteStorage::ByteStorage(const byte *bytes, size_t length, bool no_copy) : m_storedLength(0), m_no_copy(no_copy) {
        if (m_no_copy) {
            m_storage.first = const_cast<byte *>(bytes);
            m_storage.second = 0;
            m_storedLength = length;
        } else {
            allocateStorage(length);
            std::copy(bytes, &bytes[length], m_storage.first);
            m_storedLength = length;
        }
        m_read_ptr = m_storage.first;
    }

    ByteStorage::ByteStorage(ByteStorage &&orig) : m_storage(orig.m_storage), m_storedLength(orig.m_storedLength), m_no_copy(orig.m_no_copy) {
        orig.m_storage.first = nullptr;
        orig.m_storage.second = 0;
        orig.m_storedLength = 0;
        orig.m_no_copy = true;
        orig.m_read_ptr = orig.m_storage.first;
        m_read_ptr = m_storage.first;
    }

    ByteStorage::~ByteStorage() noexcept {
        if (!m_no_copy) {
            storage_traits::deallocate(m_allocator, m_storage.first, m_storage.second);
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

        m_storage.first = storage_traits::allocate(m_allocator, m_storage.second);
    }

    const size_t ByteStorage::bytes(const byte **data) const {
        *data = m_storedLength > 0 ? m_storage.first : nullptr;
        return m_storedLength;
    }

    const bool ByteStorage::operator==(const ByteStorage &other) const {
        if (m_storedLength == other.m_storedLength) {
            for (size_t i = 0; i < m_storedLength; i++) {
                if (m_storage.first[i] != other.m_storage.first[i]) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    void ByteStorage::mark() const noexcept {
        m_mark_ptr = m_read_ptr;
    }

    void ByteStorage::resetRead() const noexcept {
        m_read_ptr = m_mark_ptr;
    }

    void ByteStorage::advanceRead(const size_t distance) const {
        std::advance(m_read_ptr, distance);
    }

    const size_t ByteStorage::remainingReadLength() const {
        size_t v =  m_storedLength - (m_read_ptr - m_storage.first);
        return v;
    }

    const size_t ByteStorage::bytesRead() const {
        return m_read_ptr - m_storage.first;
    }

    const byte *ByteStorage::readBytes() const {
        return m_read_ptr;
    }

    const byte *ByteStorage::operator*() const {
        return this->readBytes();
    }

    const ByteStorage ByteStorage::segment(const size_t length) const {
        assert(length <= remainingReadLength());
        const byte *ptr = m_read_ptr;
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