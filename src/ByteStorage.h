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

        explicit ByteStorage(const size_t allocation = 256) noexcept;

    public:
        explicit ByteStorage(const byte *bytes, size_t length, bool nocopy=false) noexcept;
        ByteStorage(ByteStorage &&orig) noexcept;

        ByteStorage(const ByteStorage &) = delete;
        const ByteStorage &operator=(const ByteStorage &) = delete;

        virtual ~ByteStorage() noexcept;

        const size_t bytes(const byte **data) const noexcept;
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

        const bool operator==(const ByteStorage &other) const noexcept;

        friend std::ostream &operator<<(std::ostream &out, const ByteStorage &msg);
    };
}

#endif //TFDCF_BYTESTORAGE_H
