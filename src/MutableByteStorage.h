//
// Created by Tom Fewster on 17/02/2016.
//

#ifndef TFDCF_MUTABLEBYTESTORAGE_H
#define TFDCF_MUTABLEBYTESTORAGE_H


#include <stddef.h>
#include <utility>
#include <limits>
#include "ByteStorage.h"

namespace DCF {
    template <typename T, typename Allocator = std::allocator<T>> class MutableByteStorage : public ByteStorage<T, Allocator> {
    public:

        explicit MutableByteStorage(const size_t allocation = 256, const Allocator &allocator = Allocator()) noexcept : ByteStorage<T, Allocator>(allocation, allocator) {}
        explicit MutableByteStorage(const T *bytes, size_t length, const Allocator &allocator = Allocator()) noexcept : ByteStorage<T, Allocator>(bytes, length, false, allocator) {}

        MutableByteStorage(MutableByteStorage<T, Allocator> &&orig) noexcept : ByteStorage<T, Allocator>(std::move(orig)) {};

        virtual ~MutableByteStorage() noexcept {};

        void setData(const T *data, const size_t length) noexcept {
            if (length > this->m_storage.second) {
                ByteStorage<T, Allocator>::storage_traits::deallocate(this->m_allocator, this->m_storage.first, this->m_storage.second);
                this->allocateStorage(std::max(length, this->m_storage.second * 2));
            }
            std::copy(data, &data[length], this->m_storage.first);
            this->m_storedLength = length;
        }

        void increaseLengthBy(const size_t length) noexcept {
            if (this->m_storedLength + length > this->m_storage.second) {
                T *old_data = this->m_storage.first;
                const size_t old_length = this->m_storage.second;
                this->allocateStorage(this->m_storage.second + length);
                memmove(this->m_storage.first, old_data, old_length);
                ByteStorage<T, Allocator>::storage_traits::deallocate(this->m_allocator, old_data, old_length);
            }
            this->m_storedLength += length;
        }

        const size_t capacity() const noexcept {
            return this->m_storage.second;
        }

        void truncate(const size_t length) noexcept {
            this->m_storedLength = length;
        }

        T *mutableBytes() const noexcept {
            return this->m_storage.first;
        }

        void append(const T *buffer, const size_t length) noexcept {
            const size_t current_length = this->m_storedLength;
            this->increaseLengthBy(length);
            std::copy(buffer, &buffer[length], &this->m_storage.first[current_length]);
        }

        void append(const ByteStorage<T, Allocator> &src, const size_t length = std::numeric_limits<size_t>::max()) noexcept {
            const size_t current_length = this->m_storedLength;
            const T *data = nullptr;
            const size_t copy_length = std::min(src.bytes(&data), length);
            this->increaseLengthBy(copy_length);

            std::copy(data, &data[copy_length], &this->m_storage.first[current_length]);
        }

        void clear() noexcept {
            this->m_storedLength = 0;
            this->m_read_ptr = this->m_mark_ptr = this->m_storage.first;
        }
    };
}

#endif //TFDCF_MUTABLEBYTESTORAGE_H
