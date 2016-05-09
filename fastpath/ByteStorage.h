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

#ifndef FASTPATH_BYTESTORAGE_H
#define FASTPATH_BYTESTORAGE_H

#include <ostream>
#include <iomanip>
#include <memory>

namespace fp {
    template<typename T> T endianScalar(T t) {
#ifdef __BIG_ENDIAN__
        if (sizeof(T) == 1) {   // Compile-time if-then's.
            return t;
        } else if (sizeof(T) == 2) {
            auto r = __builtin_bswap16(*reinterpret_cast<uint16_t *>(&t));
            return *reinterpret_cast<T *>(&r);
        } else if (sizeof(T) == 4) {
            auto r = __builtin_bswap32(*reinterpret_cast<uint32_t *>(&t));
            return *reinterpret_cast<T *>(&r);
        } else if (sizeof(T) == 8) {
            auto r = __builtin_bswap64(*reinterpret_cast<uint64_t *>(&t));
            return *reinterpret_cast<T *>(&r);
        } else {
            assert(0);
        }
#else
        return t;
#endif
    }

    template <typename T, typename Allocator = std::allocator<T>> class ByteStorage {
    public:
        typedef enum {
            TRANSIENT = 1 << 0,
            COPY_ON_CONSTRUCT = 1 << 1,
            TAKE_OWNERSHIP = 1 << 2
        } storage_options;

    private:
        static_assert(sizeof(T) == 1, "Can't create byte buffer for sizes != 1");

    protected:
        using BufferDataType = std::pair<T *, size_t>;
        using storage_alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<T>;
        using storage_traits = typename std::allocator_traits<Allocator>::template rebind_traits<T>;

        storage_alloc m_allocator;

        BufferDataType m_storage;
        size_t m_storedLength;

        int m_options;

        mutable T *m_read_ptr;
        mutable T *m_mark_ptr;

        void allocateStorage(const size_t length) {
            // this will find the next x^2 number larger than the one provided
            m_storage.second = length;
            m_storage.second--;
            m_storage.second |= m_storage.second >> 1;
            m_storage.second |= m_storage.second >> 2;
            m_storage.second |= m_storage.second >> 4;
            m_storage.second |= m_storage.second >> 8;
            m_storage.second |= m_storage.second >> 16;
            m_storage.second++;

            m_storage.first = static_cast<T *>(storage_traits::allocate(m_allocator, m_storage.second));
        }

        explicit ByteStorage(const size_t allocation = 256, const Allocator &allocator = Allocator()) noexcept : m_allocator(allocator), m_storedLength(0), m_options(0) {
            allocateStorage(allocation);
            m_read_ptr = m_storage.first;
            m_mark_ptr = m_read_ptr;
        }


    public:

        explicit ByteStorage(const T *bytes, size_t length, int options = COPY_ON_CONSTRUCT | TAKE_OWNERSHIP, const Allocator &allocator = Allocator()) noexcept : m_allocator(allocator), m_storedLength(0), m_options(options) {
            if ((m_options & COPY_ON_CONSTRUCT) == COPY_ON_CONSTRUCT) {
                allocateStorage(length);
                std::copy(bytes, &bytes[length], m_storage.first);
                m_storedLength = length;
            } else {
                m_storage.first = const_cast<T *>(bytes);
                m_storage.second = 0;
                m_storedLength = length;
            }
            m_read_ptr = m_storage.first;
            m_mark_ptr = m_read_ptr;
        }

        ByteStorage(ByteStorage &&orig) noexcept : m_allocator(orig.m_allocator), m_storage(orig.m_storage), m_storedLength(orig.m_storedLength), m_options(orig.m_options) {
            orig.m_storage.first = nullptr;
            orig.m_storage.second = 0;
            orig.m_storedLength = 0;
            orig.m_options = 0;
            orig.m_read_ptr = orig.m_storage.first;
            orig.m_mark_ptr = 0;
            m_read_ptr = m_storage.first;
            m_mark_ptr = m_read_ptr;
        };

        ByteStorage(const ByteStorage &) = delete;
        const ByteStorage &operator=(const ByteStorage &) = delete;

        virtual ~ByteStorage() noexcept {
            if ((m_options & TAKE_OWNERSHIP) == TAKE_OWNERSHIP) {
                storage_traits::deallocate(m_allocator, m_storage.first, m_storage.second);
            }
        }

        /**
         * Release storage to caller, who will now take responsibility for freeing it
         */
        void release_ownership() noexcept {
            m_options &= ~TAKE_OWNERSHIP;
            m_storage.second = 0;
        }

        inline const size_t bytes(const T **data) const noexcept  {
            *data = m_storedLength > 0 ? m_storage.first : nullptr;
            return m_storedLength;
        };

        inline const size_t length() const noexcept { return m_storedLength; }

        inline const bool owns_copy() const noexcept { return (m_options & TAKE_OWNERSHIP) == TAKE_OWNERSHIP; }

        inline const T operator[](const size_t index) const noexcept {
            return m_storage.first[index];
        }

        // for reading as a stream
        inline void mark() const noexcept {
            m_mark_ptr = m_read_ptr;
        };

        inline void resetRead() const noexcept {
            m_read_ptr = m_mark_ptr;
        }

        inline void advanceRead(const size_t distance) const noexcept {
            std::advance(m_read_ptr, distance);
        }

        inline const size_t remainingReadLength() const noexcept {
            size_t v =  m_storedLength - this->bytesRead();
            return v;
        }

        inline const size_t bytesRead() const noexcept {
            return m_read_ptr - m_storage.first;
        }

        inline const T *readBytes() const noexcept {
            return m_read_ptr;
        }

        template<typename S> inline S readScalar() const noexcept {
            auto p = m_read_ptr;
            std::advance(m_read_ptr, sizeof(S));
            return endianScalar(*reinterpret_cast<const S *>(p));
        }

        const T *operator*() const noexcept {
            return this->readBytes();
        }


        const ByteStorage segment(const size_t length) const noexcept {
            assert(length <= remainingReadLength());
            auto ptr = m_read_ptr;
            advanceRead(length);
            return ByteStorage(ptr, length, true);
        };

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

        friend std::ostream &operator<<(std::ostream &out, const ByteStorage &msg) {
            const T *data = nullptr;
            const size_t length = msg.bytes(&data);
            out << "[length: " << length << " capacity: " << msg.m_storage.second << "]: " << std::endl;

            const T *output = nullptr;
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
    };
}

#endif //FASTPATH_BYTESTORAGE_H
