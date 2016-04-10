//
// Created by Tom Fewster on 23/02/2016.
//

#ifndef TFDCF_BYTESTORAGE_H
#define TFDCF_BYTESTORAGE_H

#include <ostream>
#include <memory>

namespace DCF {
    template <typename T, typename Allocator = std::allocator<T>> class ByteStorage {
        static_assert(sizeof(T) == 1, "Can'y create byte buffer for sizes != 1");
    protected:
        using BufferDataType = std::pair<T *, size_t>;
        using storage_alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<T>;
        using storage_traits = typename std::allocator_traits<Allocator>::template rebind_traits<T>;

        storage_alloc m_allocator;

        BufferDataType m_storage;
        size_t m_storedLength;

        bool m_no_copy;

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

            m_storage.first = storage_traits::allocate(m_allocator, m_storage.second);
        }

        explicit ByteStorage(const size_t allocation = 256, const Allocator &allocator = Allocator()) noexcept : m_allocator(allocator), m_storedLength(0), m_no_copy(false), m_read_ptr(m_storage.first), m_mark_ptr(m_read_ptr) {
            allocateStorage(allocation);
        }


    public:
        explicit ByteStorage(const T *bytes, size_t length, bool no_copy=false, const Allocator &allocator = Allocator()) noexcept : m_allocator(allocator), m_storedLength(0), m_no_copy(no_copy) {
            if (m_no_copy) {
                m_storage.first = const_cast<T *>(bytes);
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

        ByteStorage(ByteStorage &&orig) noexcept : m_allocator(orig.m_allocator), m_storage(orig.m_storage), m_storedLength(orig.m_storedLength), m_no_copy(orig.m_no_copy) {
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

        const size_t bytes(const T **data) const noexcept  {
            *data = m_storedLength > 0 ? m_storage.first : nullptr;
            return m_storedLength;
        };

        const size_t length() const noexcept { return m_storedLength; }

        const bool owns_copy() const noexcept { return !m_no_copy; }

        const T operator[](const size_t index) const noexcept {
            return m_storage.first[index];
        }

        // for reading as a stream
        void mark() const noexcept {
            m_mark_ptr = m_read_ptr;
        };

        void resetRead() const noexcept {
            m_read_ptr = m_mark_ptr;
        }

        void advanceRead(const size_t distance) const noexcept {
            std::advance(m_read_ptr, distance);
        }

        const size_t remainingReadLength() const noexcept {
            size_t v =  m_storedLength - this->bytesRead();
            return v;
        }

        const size_t bytesRead() const noexcept {
            return m_read_ptr - m_storage.first;
        }

        const T *readBytes() const noexcept {
            return m_read_ptr;
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

#endif //TFDCF_BYTESTORAGE_H
