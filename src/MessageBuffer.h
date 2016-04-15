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

#ifndef TFDCF_MESSAGEBUFFER2_H
#define TFDCF_MESSAGEBUFFER2_H

#include <cassert>
#include <iomanip>
#include <cstring>

#include "MutableByteStorage.h"
#include "types.h"
#include "utils/logger.h"

namespace DCF {
    struct MsgAddressing {
        using addressing_start = uint8_t;
        using msg_length = uint64_t;
        using flags = uint8_t;
        using reserved = uint32_t;
        using subject_length = uint16_t;

        static constexpr const size_t msg_length_offset() {
            return sizeof(addressing_start)
                   + sizeof(msg_length);
        }

        static constexpr const size_t size() {
            return sizeof(addressing_start)
                   + sizeof(msg_length)
                   + sizeof(flags)
                   + sizeof(reserved)
                   + sizeof(subject_length);
        }
    };
//  char *subject;

    struct MsgHeader {
        using header_start = uint8_t;
        using field_count = uint32_t;    // len:4 off:11  Number of fields in main body

        static constexpr const size_t size() {
            return sizeof(header_start)
                   + sizeof(field_count);
        }
    };

    struct MsgField {
        using type = int8_t;
        using identifier_length = uint8_t;
        using data_length = uint32_t;

        static constexpr const size_t size() {
            return sizeof(type)
                   + sizeof(identifier_length)
                   + sizeof(data_length);
        }
    };
//  void *data;

    struct alignas(8) MsgMappings {
        uint16_t identifier;
        uint16_t name_length;
    };
//  char *name;

    class MessageBuffer {
    private:
        size_t m_startIndex = 0;

        MutableByteStorage<byte> m_storage;
        const size_t visible_length() const { return m_storage.length() > m_startIndex ? m_storage.length() - m_startIndex : 0; }

    public:

        using ByteStorageType = ByteStorage<byte>;

        explicit MessageBuffer(const size_t initialAllocation) noexcept : m_startIndex(0), m_storage(initialAllocation) {
        }

        explicit MessageBuffer(byte *buffer, const size_t initialAllocation) noexcept : m_startIndex(0), m_storage(buffer, initialAllocation) {}

        MessageBuffer(MessageBuffer &&orig) : m_startIndex(orig.m_startIndex), m_storage(std::move(orig.m_storage)) {
            orig.m_startIndex = 0;
        }

        MessageBuffer(const MessageBuffer &) = delete;
        const MessageBuffer &operator=(const MessageBuffer &) = delete;

        virtual ~MessageBuffer() noexcept {}

        byte *mutableBytes() const noexcept {
            assert(m_startIndex <= m_storage.length());
            return &m_storage.mutableBytes()[m_startIndex];
        }

        void increaseLengthBy(const size_t length) {
            allocate(length);
        }

        const ByteStorageType byteStorage() const noexcept {
            const byte *bytes = nullptr;
            m_storage.bytes(&bytes);
            return ByteStorageType(&bytes[m_startIndex], visible_length(), true);
        }

        inline byte *allocate(const size_t length) {
            // expand the length of our buffer if required
            if (m_startIndex != 0 && m_storage.length() + length > m_storage.capacity()) {
                byte *bytes = m_storage.mutableBytes();
                memmove(bytes, &bytes[m_startIndex], m_storage.length() - m_startIndex);
                m_storage.truncate(m_storage.length() - m_startIndex);
                m_startIndex = 0;
            }

            const size_t previous_length = m_storage.length();
            m_storage.increaseLengthBy(length);
            return &m_storage.mutableBytes()[previous_length];
        }

        void append(const byte *data, const size_t length) {
            m_storage.append(data, length);
        }

        inline void erase_back(const size_t length) {
            if (length > 0) {
                assert(length <= visible_length());
                m_storage.truncate(m_storage.length() - length);
            }
        }

        inline void erase_front(const size_t length) {
            assert(length <= visible_length());
            assert(length + m_startIndex <= m_storage.length());
            if (length + m_startIndex >= m_storage.length()) {
                m_startIndex = 0;
                m_storage.clear();
            } else {
                if (m_storage.length() == 0) {
                    m_startIndex = 0;
                } else {
                    m_startIndex += length;
                    assert(m_startIndex <= m_storage.length());
                }
            }
        }

        inline void append(const MessageBuffer &src, const size_t length = std::numeric_limits<size_t>::max()) noexcept {
            append(src.mutableBytes(), std::min(length, src.length()));
        }

        inline const size_t bytes(const byte **data) const noexcept {
            const byte *bytes = nullptr;
            const size_t len = m_storage.bytes(&bytes);
            *data = &bytes[m_startIndex];
            assert(m_startIndex <= m_storage.length());
            return len - m_startIndex;
        }

        inline void clear() noexcept {
            m_storage.clear();
            m_startIndex = 0;
        }

        inline const size_t length() const noexcept {
            return visible_length();
        }

        const byte operator[](const size_t index) const {
            const byte *bytes = nullptr;
            m_storage.bytes(&bytes);
            assert(m_startIndex <= m_storage.length());
            return (&bytes[m_startIndex])[index];
        }

        friend std::ostream &operator<<(std::ostream &out, const MessageBuffer &msg) {
            const byte *bytes = nullptr;
            const size_t length = msg.bytes(&bytes);
            out << "[start_index: " << msg.m_startIndex << " length: " << length << " capacity: " << (msg.m_storage.capacity() - msg.m_startIndex) << "]: " << std::endl;
            const byte *output = nullptr;

            const size_t default_block = 8;
            size_t inc = 0;
            for (size_t i = 0; i < length; i += default_block) {
                inc = std::min(default_block, length - i);

                output = &bytes[i];

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

#endif //TFDCF_MESSAGEBUFFER2_H
