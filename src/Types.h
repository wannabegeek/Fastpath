//
// Created by Tom Fewster on 12/02/2016.
//

#ifndef TFDCF_TYPES_H
#define TFDCF_TYPES_H

#include <cstdint>

typedef float float32_t;
typedef double float64_t;

typedef uint8_t byte;

namespace DCF {
    typedef enum {
        unknown,
        string,
        boolean,
        uint8,
        uint16,
        uint32,
        uint64,
        int8,
        int16,
        int32,
        int64,
        float16,
        float32,
        float64,
        data,
        message
    } StorageType;

    typedef enum {
        Default = 0,
        Administration = 1 << 0,
        Garuanted = 1 << 1
    } MessageFlags;

}

#endif //TFDCF_TYPES_H
