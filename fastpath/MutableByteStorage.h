/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef FASTPATH_MUTABLEBYTESTORAGE_H
#define FASTPATH_MUTABLEBYTESTORAGE_H


#include <stddef.h>
#include <utility>
#include <limits>

#include "fastpath/utils/optimize.h"
#include "fastpath/ByteStorage.h"

namespace fp {
    template <typename T, typename Allocator = std::allocator<T>> class MutableByteStorage : public ByteStorage<T, Allocator> {
    public:

        explicit MutableByteStorage(const size_t allocation = 256, const Allocator &allocator = Allocator()) noexcept : ByteStorage<T, Allocator>(allocation, allocator) {}
        explicit MutableByteStorage(const T *bytes, size_t length, const Allocator &allocator = Allocator()) noexcept : ByteStorage<T, Allocator>(bytes, length, false, allocator) {}

        MutableByteStorage(MutableByteStorage<T, Allocator> &&orig) noexcept : ByteStorage<T, Allocator>(std::move(orig)) {};

        virtual ~MutableByteStorage() noexcept {};

        void setData(const T *data, const size_t length) noexcept {
            if (tf::unlikely(length > this->m_storage.second)) {
                ByteStorage<T, Allocator>::storage_traits::deallocate(this->m_allocator, this->m_storage.first, this->m_storage.second);
                this->allocateStorage(std::max(length, this->m_storage.second * 2));
            }
            std::copy(data, &data[length], this->m_storage.first);
            this->m_storedLength = length;
        }

        T *allocate(const size_t length) noexcept {
            if (tf::unlikely(this->m_storedLength + length > this->m_storage.second)) {
                T *old_data = this->m_storage.first;
                const size_t old_length = this->m_storage.second;
                this->allocateStorage(this->m_storage.second + length);
                memmove(this->m_storage.first, old_data, old_length);
                ByteStorage<T, Allocator>::storage_traits::deallocate(this->m_allocator, old_data, old_length);
            }

            const size_t pref_length = this->m_storedLength;
            this->m_storedLength += length;
            return &this->m_storage.first[pref_length];
        }

        inline const size_t capacity() const noexcept {
            return this->m_storage.second;
        }

        inline void truncate(const size_t length) noexcept {
            this->m_storedLength = length;
        }

        inline T *mutableBytes() const noexcept {
            return this->m_storage.first;
        }

        inline void append(const T *buffer, const size_t length) noexcept {
            T *s = this->allocate(length);
            std::copy(buffer, &buffer[length], s);
        }

        inline void append(const ByteStorage<T, Allocator> &src, const size_t length = std::numeric_limits<size_t>::max()) noexcept {
            const T *data = nullptr;
            const size_t copy_length = std::min(src.bytes(&data), length);

            this->append(data, copy_length);
        }

        template<typename S> inline void appendScalar(S s) {
            T *t = this->allocate(sizeof(S));
            *reinterpret_cast<S *>(t) = endianScalar(s);
        }

        inline void clear() noexcept {
            this->m_storedLength = 0;
            this->m_read_ptr = this->m_mark_ptr = this->m_storage.first;
        }
    };
}

#endif //FASTPATH_MUTABLEBYTESTORAGE_H
