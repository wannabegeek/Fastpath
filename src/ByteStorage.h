//
// Created by Tom Fewster on 23/02/2016.
//

#ifndef TFDCF_BYTESTORAGE_H
#define TFDCF_BYTESTORAGE_H

#include <ostream>
#include <memory>
#include "types.h"

namespace DCF {
    class ByteStorage {
    protected:
        using BufferDataType = std::pair<byte *, size_t>;
        using storage_alloc = std::allocator_traits<std::allocator<byte>>::template rebind_alloc<byte>;
        using storage_traits = std::allocator_traits<std::allocator<byte>>::template rebind_traits<byte>;

        BufferDataType m_storage;
        size_t m_storedLength;

        storage_alloc m_allocator;

        bool m_no_copy;

        mutable byte *m_read_ptr;
        mutable byte *m_mark_ptr;

        void allocateStorage(const size_t length);

        explicit ByteStorage(const size_t allocation = 256) noexcept : m_storedLength(0), m_no_copy(false), m_read_ptr(m_storage.first), m_mark_ptr(m_read_ptr) {
            allocateStorage(allocation);
        }


    public:
        explicit ByteStorage(const byte *bytes, size_t length, bool nocopy=false) noexcept : m_storedLength(0), m_no_copy(no_copy) {
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
            m_mark_ptr = m_read_ptr;
        }

        ByteStorage(ByteStorage &&orig) noexcept : m_storage(orig.m_storage), m_storedLength(orig.m_storedLength), m_no_copy(orig.m_no_copy) {
            orig.m_storage.first = nullptr;
            orig.m_storage.second = 0;
            orig.m_storedLength = 0;
            orig.m_no_copy = true;
            orig.m_read_ptr = orig.m_storage.first;
            orig.m_mark_ptr = 0;
            m_read_ptr = m_storage.first;
            m_mark_ptr = m_read_ptr;
        };

        ByteStorage(const ByteStorage &) = delete;
        const ByteStorage &operator=(const ByteStorage &) = delete;

        virtual ~ByteStorage() noexcept {
            if (!m_no_copy) {
                storage_traits::deallocate(m_allocator, m_storage.first, m_storage.second);
            }
        }

        const size_t bytes(const byte **data) const noexcept  {
            *data = m_storedLength > 0 ? m_storage.first : nullptr;
            return m_storedLength;
        };

        const size_t length() const noexcept { return m_storedLength; }

        const bool owns_copy() const noexcept { return !m_no_copy; }

        const byte operator[](const size_t index) const noexcept {
            return m_storage.first[index];
        }

        // for reading as a stream
        void mark() const noexcept;
        void resetRead() const noexcept;
        void advanceRead(const size_t distance) const noexcept;
        const size_t remainingReadLength() const noexcept;
        const size_t bytesRead() const noexcept;
        const byte *readBytes() const noexcept;
        const byte *operator*() const noexcept;

        const ByteStorage segment(const size_t length) const noexcept;

        const bool operator==(const ByteStorage &other) const noexcept {
            if (m_storedLength == other.m_storedLength) {
                for (size_t i = 0; i < m_storedLength; i++) {
                    if (m_storage.first[i] != other.m_storage.first[i]) {
                        return false;
                    }
                }
                return true;
            }
            return false;
        };

        friend std::ostream &operator<<(std::ostream &out, const ByteStorage &msg);
    };
}

#endif //TFDCF_BYTESTORAGE_H
