//
// Created by Tom Fewster on 12/02/2016.
//

#ifndef TFDCF_STORAGETYPES_H
#define TFDCF_STORAGETYPES_H

#include <cstdint>
#include <memory>

namespace DCF {
    typedef enum {
        unknown,
        string,
        date_time,
        boolean,
        uint8,
        uint16,
        uint32,
        uint64,
        int8,
        int16,
        int32,
        int64,
//        float16,
        float32,
        float64,
        data,
        message
    } StorageType;

    static constexpr const char *StorageTypeDescription[16] = {
            "unknown",
            "string",
            "date_time",
            "boolean",
            "uint8",
            "uint16",
            "uint32",
            "uint64",
            "int8",
            "int16",
            "int32",
            "int64",
//        float16,
            "float32",
            "float64",
            "data",
            "message"
    };

    typedef enum {
        Default = 0,
        Administration = 1 << 0,
        Garuanted = 1 << 1
    } MessageFlags;

    class Message;

    using MessageType = std::shared_ptr<Message>;
}

#endif //TFDCF_STORAGETYPES_H
