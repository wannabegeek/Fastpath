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


#include <stdint.h>

namespace DCF {

    struct Header {
        uint64_t msg_length;     // len:8 off:0   Message Length
        uint8_t flags;           // len:1 off:8   Flags
        uint32_t reserved;       // len:4 off:9   Reserved for future use
        uint32_t field_count;    // len:4 off:11  Number of fields in main body
        uint16_t subject_length; // len:2 off:15  Length of subject
        char *subject;          // len:? off:17  Ptr to first char of subject
    };

    struct Fields {
        uint16_t identifier;
        uint32_t offset;
        uint16_t name_length;
        char *name;
    };

    struct Data {
        int8_t type;
        uint32_t length;
        void *data;
    };

    class MessageBuffer {

    };
}

#endif //TFDCF_MESSAGEBUFFER_H
