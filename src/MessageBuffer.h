//
// Created by Tom Fewster on 15/02/2016.
//

#ifndef TFDCF_MESSAGEBUFFER_H
#define TFDCF_MESSAGEBUFFER_H

/**
 *
 * Header
 * | Msg Length | Flags | Reserved  | Subject Length | Subject |
 * |   8 bytes  |  1 b  |  16 bytes |   4 bytes      |  Var    |
 *
 * Field Map Repeating Block
 * | Num Fields |
 * |  4 bytes   |
 *
 *      | Identifier | Offset  | Name Length | Name |
 *      |  2 bytes   | 8 bytes |  2 bytes    |  Var |
 *
 * Data Segment Repeating Block
 *      | Data Type | Field Length | Data |
 *      |   1 byte  |  8 bytes     |  Var |
 *
 */


#include <cstdint>
#include <memory>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iomanip>

#include "Types.h"

namespace DCF {

    struct alignas(16) MsgHeader {
        uint64_t msg_length;     // len:8 off:0   Message Length
        uint8_t flags;           // len:1 off:8   Flags
        uint32_t reserved;       // len:4 off:9   Reserved for future use
        uint32_t field_count;    // len:4 off:11  Number of fields in main body
        uint32_t mapping_count;  // len:4 off:15  Number of fields in main body
        uint16_t subject_length; // len:2 off:19  Length of subject
    };
//  char *subject;

    struct alignas(16) MsgField {
        uint16_t identifier;
        int8_t type;
        uint32_t data_length;
    };
//  void *data;

    struct alignas(16) MsgMappings {
        uint16_t identifier;
        uint16_t name_length;
    };
//  char *name;

    class MessageBuffer {
    private:
        byte *m_buffer = nullptr;
        size_t m_bufferLength = 0;
        size_t m_msgLength = 0;
        size_t m_startIndex = 0;

    public:
        using BufferDataType = std::pair<const byte *, size_t>;

        MessageBuffer(const size_t initialAllocation) noexcept : m_msgLength(0), m_startIndex(0)  {
            // this will find the next x^2 number larger than the one provided
            m_bufferLength = initialAllocation;
            m_bufferLength--;
            m_bufferLength |= m_bufferLength >> 1;
            m_bufferLength |= m_bufferLength >> 2;
            m_bufferLength |= m_bufferLength >> 4;
            m_bufferLength |= m_bufferLength >> 8;
            m_bufferLength |= m_bufferLength >> 16;
            m_bufferLength++;

            m_buffer = new byte[m_bufferLength];
        }

        MessageBuffer(const size_t initialAllocation, byte *buffer) noexcept : m_buffer(buffer), m_bufferLength(initialAllocation), m_msgLength(0), m_startIndex(0) {}

        MessageBuffer(MessageBuffer &&orig) : m_msgLength(orig.m_msgLength), m_startIndex(orig.m_startIndex) {
            m_buffer = orig.m_buffer;
            orig.m_buffer = new byte[orig.m_bufferLength];
            m_bufferLength = orig.m_bufferLength;
            orig.m_msgLength = 0;
            orig.m_startIndex = 0;
        }

        MessageBuffer(const MessageBuffer &) = delete;
        const MessageBuffer &operator=(const MessageBuffer &) = delete;

        ~MessageBuffer() noexcept {
            delete [] m_buffer;
        }

        inline const byte *append(const byte *buffer, const size_t length) noexcept {
            byte *data = nullptr;
            if (length) {
                // copy our buffer into place
                data = allocate(length);
                memcpy(data, buffer, length);
            }
            return data;
        }

        inline byte *allocate(const size_t length) {
            // expand the length of our buffer if required
            if (m_startIndex + m_msgLength + length >= m_bufferLength) {
                size_t newLength = m_bufferLength * 2;
                while (m_msgLength + length > newLength) {
                    newLength *= 2;
                }
                byte *tmp = new byte[newLength];
                memmove(tmp, &m_buffer[m_startIndex], m_msgLength);
                delete [] m_buffer;
                m_buffer = tmp;
                m_startIndex = 0;
                m_bufferLength = newLength;
            }
            byte *result = &m_buffer[m_startIndex + m_msgLength];
            m_msgLength += length;

            return result;
        }

        inline void erase_back(const size_t length) {
            assert(length <= m_msgLength);
            m_msgLength -= length;
        }

        inline void erase_front(const size_t length) {
            assert(length <= m_msgLength);
            m_msgLength -= length;
            if (m_msgLength == 0) {
                m_startIndex = 0;
            } else {
                m_startIndex += length;
            }
        }

        inline void advance(const size_t length = 0) noexcept {
            byte *data = nullptr;
            if (length) {
                // copy our buffer into place
                data = allocate(length);
            }
        }

        inline void append(const MessageBuffer &src, const size_t length = 0) noexcept {
            if (length != 0) {
                append(&(src.m_buffer[src.m_startIndex]), length);
            } else {
                append(&(src.m_buffer[src.m_startIndex]), src.m_msgLength);
            }
        }

        inline void clear() noexcept {
            m_msgLength = 0;
            m_startIndex = 0;
        }

        inline const size_t size() const noexcept {
            return m_msgLength;
        }

        inline const BufferDataType data() const noexcept {
            return std::move(std::make_pair(&m_buffer[m_startIndex], m_msgLength));
        }

        const byte operator[](const size_t index) const {
            return (&m_buffer[m_startIndex])[index];
        }

        friend std::ostream &operator<<(std::ostream &out, const MessageBuffer &msg) {
            const BufferDataType &data = msg.data();
            out << "[start_index: " << msg.m_startIndex << " length: " << msg.m_msgLength << " capacity: " << msg.m_bufferLength << "]: " << std::endl;
            const byte *output = nullptr;

            const size_t default_block = 8;
            size_t inc = 0;

            for (int i = 0; i < data.second; i += default_block) {
                inc = std::min(default_block, data.second - i);

                output = &data.first[i];

                out << std::setfill('0') << std::setw(5) << i << "   ";

                for (int j = 0; j < default_block; j++) {
                    if (j < inc) {
                        out << std::setfill('0') << std::setw(2) << std::hex << static_cast<const int>(output[j]) << " ";
                    } else {
                        out << "   ";
                    }
                }
                out << std::dec << "        ";

                for (int j = 0; j < default_block; j++) {
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

#endif //TFDCF_MESSAGEBUFFER_H
